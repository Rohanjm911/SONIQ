#include "LibraryScanner.hpp"
#include "metadata/AudioProbe.hpp"
#include "metadata/ArtworkExtractor.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <algorithm>

namespace soniq::library {

LibraryScanner::LibraryScanner(QObject* parent)
    : QObject(parent) {}

LibraryScanner::~LibraryScanner() {
    cancel();
}

void LibraryScanner::cancel() {
    m_cancelRequested = true;
    if (m_workerThread && m_workerThread->joinable()) {
        m_workerThread->join();
    }
    m_scanning = false;
}

void LibraryScanner::scanDirectory(const QString& directoryPath) {
    if (m_scanning) return;

    cancel();
    m_scanning = true;
    m_cancelRequested = false;

    m_workerThread = std::make_unique<std::thread>(&LibraryScanner::doScan, this, directoryPath);
}

void LibraryScanner::doScan(const QString& path) {
    emit scanStarted(path);

    namespace fs = std::filesystem;
    std::string stdPath = path.toStdString();
    std::vector<fs::path> audioFiles;

    std::error_code ec;
    if (fs::exists(stdPath, ec) && fs::is_directory(stdPath, ec)) {
        for (const auto& entry : fs::recursive_directory_iterator(stdPath, fs::directory_options::skip_permission_denied, ec)) {
            if (m_cancelRequested) break;
            if (entry.is_regular_file(ec)) {
                std::string ext = entry.path().extension().string();
                if (metadata::AudioProbe::isFormatSupported(ext)) {
                    audioFiles.push_back(entry.path());
                }
            }
        }
    }

    const int total = static_cast<int>(audioFiles.size());
    int addedCount = 0;

    for (int i = 0; i < total; ++i) {
        if (m_cancelRequested) break;

        const auto& filePath = audioFiles[i].string();
        emit scanProgress(i + 1, total, QString::fromStdString(filePath));

        auto inspection = metadata::AudioProbe::probeFile(filePath);
        if (inspection.success) {
            db::TrackRecord rec;
            rec.filePath = filePath;
            rec.title = inspection.metadata.title;
            rec.artist = inspection.metadata.artist;
            rec.album = inspection.metadata.album;
            rec.albumArtist = inspection.metadata.albumArtist;
            rec.genre = inspection.metadata.genre;
            rec.year = inspection.metadata.year;
            rec.trackNumber = inspection.metadata.trackNumber;
            rec.discNumber = inspection.metadata.discNumber;
            rec.durationSeconds = inspection.metadata.durationSeconds;
            rec.sampleRate = inspection.format.sampleRate();
            rec.bitDepth = inspection.format.bitDepth();
            rec.channels = inspection.format.channels();
            rec.bitrate = inspection.format.bitrate();
            rec.format = inspection.format.codec();
            rec.fileSize = inspection.metadata.fileSize;
            rec.artworkPath = metadata::ArtworkExtractor::instance().getOrExtractArtwork(filePath);

            if (db::DatabaseManager::instance().insertOrUpdateTrack(rec) > 0) {
                addedCount++;
            }
        }
    }

    m_scanning = false;
    emit scanFinished(addedCount, total);
}

} // namespace soniq::library
