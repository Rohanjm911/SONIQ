#include "audio/AudioPlaybackEngine.hpp"
#include "audio/PlaybackQueue.hpp"
#include "database/DatabaseManager.hpp"
#include "library/LibraryScanner.hpp"
#include "metadata/ArtworkExtractor.hpp"
#include "services/LibraryModel.hpp"
#include "services/PlaybackController.hpp"
#include <QCoreApplication>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

using namespace soniq;

void testArtworkCacheAndKey() {
  std::cout << "Testing ArtworkExtractor cache directory & key generation..."
            << std::endl;

  auto &extractor = metadata::ArtworkExtractor::instance();
  std::string cacheDir = extractor.cacheDirectory();
  assert(!cacheDir.empty() && "Artwork cache directory should not be empty");

  // Test with non-existent audio file
  std::string noFile = extractor.getOrExtractArtwork("non_existent_file.flac");
  assert(noFile.empty() &&
         "Non-existent file should return empty artwork string");

  std::cout << "  -> ArtworkExtractor cache and key generation PASSED."
            << std::endl;
}

void testQueueLookaheadMethods() {
  std::cout << "Testing PlaybackQueue peekNext() and advanceNext()..."
            << std::endl;

  audio::PlaybackQueue q;
  audio::QueueItem item1;
  item1.filePath = "track1.flac";
  item1.title = "Track One";

  audio::QueueItem item2;
  item2.filePath = "track2.flac";
  item2.title = "Track Two";

  audio::QueueItem item3;
  item3.filePath = "track3.flac";
  item3.title = "Track Three";

  q.append(item1);
  q.append(item2);
  q.append(item3);

  assert(q.size() == 3);
  assert(q.currentIndex() == 0);
  assert(q.currentItem()->title == "Track One");

  // peekNext must return item 2 without changing currentIndex
  auto peek1 = q.peekNext();
  assert(peek1.has_value() && "peekNext should have returned track 2");
  assert(peek1->title == "Track Two");
  assert(q.currentIndex() == 0 && "peekNext must not advance currentIndex");

  // advanceNext should move to item 2
  auto adv1 = q.advanceNext();
  assert(adv1.has_value() && adv1->title == "Track Two");
  assert(q.currentIndex() == 1);
  assert(q.currentItem()->title == "Track Two");

  // peekNext now returns track 3
  auto peek2 = q.peekNext();
  assert(peek2.has_value() && peek2->title == "Track Three");
  assert(q.currentIndex() == 1);

  // advance to track 3
  auto adv2 = q.advanceNext();
  assert(adv2.has_value() && adv2->title == "Track Three");
  assert(q.currentIndex() == 2);

  // At end of queue without repeat, peekNext returns nullopt
  assert(!q.peekNext().has_value() &&
         "peekNext at end of queue without repeat should be nullopt");

  // Enable Queue repeat
  q.setRepeatMode(core::RepeatMode::Queue);
  auto peekLoop = q.peekNext();
  assert(peekLoop.has_value() && peekLoop->title == "Track One" &&
         "peekNext with RepeatMode::Queue should wrap to beginning");

  std::cout << "  -> PlaybackQueue lookahead methods PASSED." << std::endl;
}

void testDatabaseArtworkPersistence() {
  std::cout
      << "Testing database artwork_path column persistence (Migration v2)..."
      << std::endl;

  namespace fs = std::filesystem;
  fs::path testDbPath = fs::temp_directory_path() / "test_artwork_migration.db";
  std::error_code ec;
  if (fs::exists(testDbPath, ec)) {
    fs::remove(testDbPath, ec);
  }

  auto &db = db::DatabaseManager::instance();
  db.close();
  bool initOk = db.initialize(QString::fromStdWString(testDbPath.wstring()));
  assert(initOk && "Database initialization failed");

  db::TrackRecord track;
  track.filePath = "C:/Music/test_song.flac";
  track.title = "Test Song";
  track.artist = "Audiophile Artist";
  track.album = "Hi-Res Album";
  track.durationSeconds = 240.0;
  track.sampleRate = 96000;
  track.bitDepth = 24;
  track.channels = 2;
  track.format = "FLAC";
  track.artworkPath =
      "C:/Users/AppData/SONIQ/artwork_cache/abcdef1234567890.jpg";

  int64_t trackId = db.insertOrUpdateTrack(track);
  assert(trackId > 0 && "Failed to insert track with artwork_path");

  auto tracks = db.getAllTracks();
  assert(!tracks.empty() && "Tracks list empty");
  assert(tracks[0].artworkPath == track.artworkPath &&
         "Stored artwork_path does not match inserted value");

  db.close();
  fs::remove(testDbPath);
  std::cout << "  -> Database artwork persistence PASSED." << std::endl;
}

void testEngineGaplessAndCrossfadeConfiguration() {
  std::cout << "Testing AudioPlaybackEngine gapless and crossfade controls..."
            << std::endl;

  audio::AudioPlaybackEngine engine;
  assert(engine.isGaplessEnabled() && "Gapless should be enabled by default");

  engine.setGaplessEnabled(false);
  assert(!engine.isGaplessEnabled());

  assert(!engine.isCrossfadeEnabled() &&
         "Crossfade should be disabled by default");
  engine.setCrossfade(true, 3.5);
  assert(engine.isCrossfadeEnabled());
  assert(std::abs(engine.crossfadeDuration() - 3.5) < 1e-4);

  // Crossfade should cause bit-perfect status to be Unavailable
  assert(engine.bitPerfectStatus() == core::BitPerfectStatus::Unavailable);
  assert(engine.bitPerfectStatusExplanation().find("crossfading") !=
         std::string::npos);

  // Turning off crossfade restores bit-perfect eligibility
  engine.setCrossfade(false, 0.0);
  assert(!engine.isCrossfadeEnabled());

  std::cout << "  -> Engine gapless & crossfade controls PASSED." << std::endl;
}

static void createSampleWav(const std::filesystem::path& path) {
    std::ofstream out(path, std::ios::binary);
    assert(out.is_open());
    const uint32_t sampleRate = 44100;
    const uint16_t channels = 2;
    const uint16_t bitsPerSample = 16;
    const uint32_t numSamples = sampleRate * 2; // 2 seconds
    const uint32_t dataSize = numSamples * channels * (bitsPerSample / 8);
    const uint32_t fileSize = 36 + dataSize;

    char header[44] = {
        'R', 'I', 'F', 'F',
        static_cast<char>(fileSize & 0xFF), static_cast<char>((fileSize >> 8) & 0xFF),
        static_cast<char>((fileSize >> 16) & 0xFF), static_cast<char>((fileSize >> 24) & 0xFF),
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        16, 0, 0, 0,
        1, 0, // PCM
        static_cast<char>(channels & 0xFF), static_cast<char>((channels >> 8) & 0xFF),
        static_cast<char>(sampleRate & 0xFF), static_cast<char>((sampleRate >> 8) & 0xFF),
        static_cast<char>((sampleRate >> 16) & 0xFF), static_cast<char>((sampleRate >> 24) & 0xFF),
        static_cast<char>((sampleRate * channels * 2) & 0xFF), static_cast<char>(((sampleRate * channels * 2) >> 8) & 0xFF),
        static_cast<char>(((sampleRate * channels * 2) >> 16) & 0xFF), static_cast<char>(((sampleRate * channels * 2) >> 24) & 0xFF),
        4, 0, // block align
        16, 0, // 16-bit
        'd', 'a', 't', 'a',
        static_cast<char>(dataSize & 0xFF), static_cast<char>((dataSize >> 8) & 0xFF),
        static_cast<char>((dataSize >> 16) & 0xFF), static_cast<char>((dataSize >> 24) & 0xFF)
    };
    out.write(header, 44);
    std::vector<char> silence(dataSize, 0);
    out.write(silence.data(), silence.size());
    out.close();
}

void testLibraryScannerAndWorkerThread() {
  std::cout << "Testing LibraryScanner folder scanning with spaces & worker "
               "thread SQLite..."
            << std::endl;

  namespace fs = std::filesystem;
  fs::path testDbPath = fs::temp_directory_path() / "test_scanner_db.db";
  std::error_code ec;
  if (fs::exists(testDbPath, ec)) {
    fs::remove(testDbPath, ec);
  }
  auto &db = db::DatabaseManager::instance();
  db.close();
  bool dbOk = db.initialize(QString::fromStdWString(testDbPath.wstring()));
  assert(dbOk && "Database initialization failed for scanner test");

  fs::path tempDir =
      fs::temp_directory_path() / "soniq_scanner_test_dir with spaces";
  fs::create_directories(tempDir / "nested folder");

  fs::path wav1 = tempDir / "track 01.wav";
  fs::path wav2 = tempDir / "nested folder" / "track 02.wav";
  createSampleWav(wav1);
  createSampleWav(wav2);

  soniq::library::LibraryScanner scanner;
  soniq::services::LibraryModel model;

  QObject::connect(&scanner, &soniq::library::LibraryScanner::scanFinished,
                   &model, &soniq::services::LibraryModel::refresh);

  scanner.scanDirectory(QString::fromStdWString(tempDir.wstring()));

  // Wait for scanning to finish (runs on worker thread)
  int waitMs = 0;
  while (scanner.isScanning() && waitMs < 5000) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    waitMs += 50;
    QCoreApplication::processEvents();
  }

  assert(!scanner.isScanning() && "Scanner should finish within 5 seconds");
  assert(scanner.scannedCount() >= 2 && "Should have scanned at least 2 files");

  model.refresh();
  assert(model.rowCount() >= 2 && "LibraryModel should contain scanned tracks");

  // Test sorting operations
  assert(model.sortField() == "title");
  assert(model.sortAscending() == true);

  // Toggle title sort to descending
  model.sortBy("title");
  assert(model.sortAscending() == false);
  assert(model.sortLabel() == "Title (Z → A)");

  // Toggle title sort back to ascending
  model.sortBy("title");
  assert(model.sortAscending() == true);
  assert(model.sortLabel() == "Title (A → Z)");

  // Test duration sort
  model.sortBy("duration");
  assert(model.sortField() == "duration");
  assert(model.sortAscending() == true);

  // Test setSort
  model.setSort("album", true);
  assert(model.sortField() == "album");
  assert(model.sortAscending() == true);

  // Test shuffleList
  model.shuffleList();
  assert(model.sortField() == "custom");
  assert(model.sortLabel() == "Shuffled");

  // Test PlaybackController queue population from LibraryModel, next(),
  // previous()
  {
    model.sortBy("title"); // reset to title order
    soniq::audio::AudioPlaybackEngine engine;
    soniq::services::PlaybackController controller(engine);
    controller.setLibraryModel(&model);

    controller.playFile(QString::fromStdWString(wav1.wstring()));
    assert(engine.queue().size() >= 2 &&
           "Queue should be populated with all library tracks");
    assert(engine.queue().currentIndex() == 0 &&
           "Current index should be 0 for track 1");
    assert(controller.trackFilePath() ==
           QString::fromStdWString(wav1.wstring()));

    // Click Next
    controller.next();
    assert(engine.queue().currentIndex() == 1 &&
           "Current index should advance to 1 on next()");
    assert(controller.trackFilePath() ==
           QString::fromStdWString(wav2.wstring()));

    // Click Previous (within 3 seconds seeks back to 0)
    controller.previous();
    assert(engine.queue().currentIndex() == 0 &&
           "Current index should go back to 0 on previous()");
    assert(controller.trackFilePath() ==
           QString::fromStdWString(wav1.wstring()));

    engine.stop();
  }

  // Clean up temporary test files & db
  ec.clear();
  fs::remove_all(tempDir, ec);
  db.close();
  fs::remove(testDbPath, ec);

  std::cout << "  -> LibraryScanner & multi-threaded SQLite integration PASSED."
            << std::endl;
}

void runArtworkAndGaplessTests() {
  std::cout << "========================================" << std::endl;
  std::cout << " RUNNING ARTWORK & GAPLESS TEST SUITE   " << std::endl;
  std::cout << "========================================" << std::endl;

  testArtworkCacheAndKey();
  testQueueLookaheadMethods();
  testDatabaseArtworkPersistence();
  testEngineGaplessAndCrossfadeConfiguration();
  testLibraryScannerAndWorkerThread();

  std::cout << "========================================" << std::endl;
  std::cout << " ALL ARTWORK & GAPLESS TESTS PASSED (5/5)" << std::endl;
  std::cout << "========================================" << std::endl;
}
