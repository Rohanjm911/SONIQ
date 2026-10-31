#include "PlaybackController.hpp"
#include "LibraryModel.hpp"
#include "database/DatabaseManager.hpp"
#include "metadata/ArtworkExtractor.hpp"
#include "metadata/AudioProbe.hpp"
#include <cmath>
#include <filesystem>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStringConverter>

namespace soniq::services {

PlaybackController::PlaybackController(audio::AudioPlaybackEngine &engine,
                                       QObject *parent)
    : QObject(parent), m_engine(engine) {

  m_engine.setStateChangedCallback([this](core::PlaybackState) {
    QMetaObject::invokeMethod(
        this,
        [this]() {
          emit stateChanged();
          emit bitPerfectChanged();
        },
        Qt::QueuedConnection);
  });

  m_engine.setVisualizerCallback(
      [this](const audio::EngineVisualizerData &viz) {
        m_peakL.store(viz.peakLevelL, std::memory_order_relaxed);
        m_peakR.store(viz.peakLevelR, std::memory_order_relaxed);
        {
          std::lock_guard<std::mutex> lock(m_vizMutex);
          m_waveformSamples = viz.waveform;
        }
      });

  m_engine.setTrackEndedCallback([this]() {
    QMetaObject::invokeMethod(
        this, [this]() { updateCurrentTrackMetadata(); }, Qt::QueuedConnection);
  });

  connect(&m_pollTimer, &QTimer::timeout, this, [this]() {
    emit positionChanged();
    emit visualizerChanged();
  });
  m_pollTimer.start(50); // 20 Hz safe GUI thread updates
}

PlaybackController::~PlaybackController() = default;

bool PlaybackController::isPlaying() const noexcept {
  return m_engine.state() == core::PlaybackState::Playing;
}

bool PlaybackController::isPaused() const noexcept {
  return m_engine.state() == core::PlaybackState::Paused;
}

QString PlaybackController::trackTitle() const { return m_title; }

QString PlaybackController::trackArtist() const { return m_artist; }

QString PlaybackController::trackAlbum() const { return m_album; }

QString PlaybackController::trackFormat() const { return m_format; }

QString PlaybackController::trackFilePath() const { return m_filePath; }

QString PlaybackController::trackArtworkPath() const { return m_artworkPath; }

double PlaybackController::position() const noexcept {
  return m_engine.currentPositionSeconds();
}

double PlaybackController::duration() const noexcept {
  return m_engine.totalDurationSeconds();
}

QString PlaybackController::formatTime(double seconds) {
  if (seconds <= 0.0)
    return "0:00";
  int totalSec = static_cast<int>(seconds);
  int mins = totalSec / 60;
  int secs = totalSec % 60;
  int hours = mins / 60;
  mins %= 60;

  if (hours > 0) {
    return QString::asprintf("%d:%02d:%02d", hours, mins, secs);
  }
  return QString::asprintf("%d:%02d", mins, secs);
}

QString PlaybackController::positionFormatted() const {
  return formatTime(position());
}

QString PlaybackController::durationFormatted() const {
  return formatTime(duration());
}

float PlaybackController::volume() const noexcept { return m_engine.volume(); }

void PlaybackController::setVolume(float vol) {
  if (std::abs(m_engine.volume() - vol) > 1e-4f) {
    m_engine.setVolume(vol);
    emit volumeChanged();
    emit bitPerfectChanged();
  }
}

bool PlaybackController::isMuted() const noexcept { return m_engine.isMuted(); }

void PlaybackController::setMuted(bool mute) {
  if (m_engine.isMuted() != mute) {
    m_engine.setMuted(mute);
    emit muteChanged();
    emit bitPerfectChanged();
  }
}

int PlaybackController::repeatMode() const noexcept {
  return static_cast<int>(m_engine.queue().repeatMode());
}

void PlaybackController::setRepeatMode(int mode) {
  auto rep = static_cast<core::RepeatMode>(mode);
  if (m_engine.queue().repeatMode() != rep) {
    m_engine.queue().setRepeatMode(rep);
    emit repeatModeChanged();
  }
}

bool PlaybackController::isShuffle() const noexcept {
  return m_engine.queue().isShuffle();
}

void PlaybackController::toggleShuffle() {
  bool current = m_engine.queue().isShuffle();
  m_engine.queue().setShuffle(!current);
  emit shuffleChanged();
}

QString PlaybackController::bitPerfectStatus() const {
  switch (m_engine.bitPerfectStatus()) {
  case core::BitPerfectStatus::Verified:
    return "Verified";
  case core::BitPerfectStatus::Available:
    return "Available";
  case core::BitPerfectStatus::Unavailable:
    return "Unavailable";
  case core::BitPerfectStatus::Unknown:
    return "Unknown";
  }
  return "Unknown";
}

QString PlaybackController::bitPerfectExplanation() const {
  return QString::fromStdString(m_engine.bitPerfectStatusExplanation());
}

void PlaybackController::play() {
  m_engine.play();
  emit stateChanged();
}

void PlaybackController::pause() {
  m_engine.pause();
  emit stateChanged();
}

void PlaybackController::togglePlayPause() {
  m_engine.togglePlayPause();
  emit stateChanged();
}

void PlaybackController::stop() {
  m_engine.stop();
  emit stateChanged();
  emit positionChanged();
}

void PlaybackController::updateCurrentTrackMetadata() {
  auto cur = m_engine.queue().currentItem();
  if (cur.has_value()) {
    m_title = QString::fromStdString(cur->title);
    m_artist = QString::fromStdString(cur->artist);
    m_album = QString::fromStdString(cur->album);
    m_filePath = QString::fromStdString(cur->filePath);
    m_artworkPath = QString::fromStdString(
        metadata::ArtworkExtractor::instance().getOrExtractArtwork(
            cur->filePath));

    auto inspection = metadata::AudioProbe::probeFile(cur->filePath);
    m_format = QString::fromStdString(inspection.format.toDetailedString());

    loadLyricsForTrack(m_filePath);

    emit trackChanged();
    emit durationChanged();
    emit stateChanged();
    emit bitPerfectChanged();
    emit queueChanged();
  }
}

QVariantList PlaybackController::waveform() const {
  std::lock_guard<std::mutex> lock(m_vizMutex);
  QVariantList list;
  list.reserve(static_cast<qsizetype>(m_waveformSamples.size()));
  for (float v : m_waveformSamples) {
    list.append(v);
  }
  return list;
}

QVariantList PlaybackController::queueItems() const {
  auto items = m_engine.queue().items();
  QVariantList list;
  list.reserve(static_cast<qsizetype>(items.size()));
  for (size_t i = 0; i < items.size(); ++i) {
    const auto& it = items[i];
    QVariantMap map;
    map["index"] = static_cast<int>(i);
    map["title"] = QString::fromStdString(it.title);
    map["artist"] = QString::fromStdString(it.artist);
    map["album"] = QString::fromStdString(it.album);
    map["filePath"] = QString::fromStdString(it.filePath);
    map["duration"] = it.durationSeconds;
    map["durationFormatted"] = formatTime(it.durationSeconds);
    map["isCurrent"] = (i == m_engine.queue().currentIndex());
    list.append(map);
  }
  return list;
}

int PlaybackController::queueIndex() const {
  return static_cast<int>(m_engine.queue().currentIndex());
}

int PlaybackController::queueCount() const {
  return static_cast<int>(m_engine.queue().size());
}

void PlaybackController::removeQueueItem(int index) {
  if (index >= 0 && index < static_cast<int>(m_engine.queue().size())) {
    m_engine.queue().remove(static_cast<size_t>(index));
    emit queueChanged();
  }
}

void PlaybackController::moveQueueItem(int fromIndex, int toIndex) {
  if (fromIndex >= 0 && fromIndex < static_cast<int>(m_engine.queue().size()) &&
      toIndex >= 0 && toIndex < static_cast<int>(m_engine.queue().size())) {
    m_engine.queue().move(static_cast<size_t>(fromIndex), static_cast<size_t>(toIndex));
    emit queueChanged();
  }
}

void PlaybackController::clearQueue() {
  m_engine.queue().clear();
  emit queueChanged();
}

void PlaybackController::playQueueIndex(int index) {
  if (index >= 0 && index < static_cast<int>(m_engine.queue().size())) {
    m_engine.queue().setCurrentIndex(static_cast<size_t>(index));
    auto cur = m_engine.queue().currentItem();
    if (cur.has_value()) {
      m_engine.playTrack(cur->filePath);
      updateCurrentTrackMetadata();
      emit queueChanged();
    }
  }
}

void PlaybackController::enqueueTrack(const QString& filePath, bool playNext) {
  std::string stdPath = filePath.toStdString();
  if (stdPath.empty()) return;

  auto inspection = metadata::AudioProbe::probeFile(stdPath);
  std::string title = !inspection.metadata.title.empty()
                          ? inspection.metadata.title
                          : std::filesystem::path(stdPath).stem().string();
  std::string artist = !inspection.metadata.artist.empty()
                           ? inspection.metadata.artist
                           : "Unknown Artist";
  std::string album = !inspection.metadata.album.empty()
                          ? inspection.metadata.album
                          : "Unknown Album";

  audio::QueueItem item;
  item.filePath = stdPath;
  item.title = title;
  item.artist = artist;
  item.album = album;
  item.durationSeconds = inspection.metadata.durationSeconds;

  if (playNext) {
    m_engine.queue().insertNext(item);
  } else {
    m_engine.queue().append(item);
  }
  emit queueChanged();
}

void PlaybackController::loadLyricsForTrack(const QString& filePath) {
  m_lyricsText.clear();
  m_syncedLyrics.clear();
  m_hasLyrics = false;
  m_isSyncedLyrics = false;

  if (filePath.isEmpty()) {
    emit lyricsChanged();
    return;
  }

  try {
    std::filesystem::path trackPath(filePath.toStdWString());
    std::filesystem::path dir = trackPath.parent_path();
    std::filesystem::path stem = trackPath.stem();

    std::vector<std::filesystem::path> candidates = {
        dir / (stem.string() + ".lrc"),
        dir / (stem.string() + ".LRC"),
        trackPath.string() + ".lrc",
        dir / (stem.string() + ".txt"),
        trackPath.string() + ".txt"
    };

    std::string foundPath;
    for (const auto& c : candidates) {
      std::error_code ec;
      if (std::filesystem::exists(c, ec) && std::filesystem::is_regular_file(c, ec)) {
        foundPath = c.string();
        break;
      }
    }

    if (!foundPath.empty()) {
      QFile file(QString::fromStdString(foundPath));
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        QString content = in.readAll();
        file.close();

        // Check for synchronized timestamps like [01:23.45] or [01:23]
        QRegularExpression re(R"(\[(\d{1,2}):(\d{2})(?:\.(\d{1,3}))?\](.*))");
        QStringList lines = content.split('\n');
        QVariantList syncedList;

        for (const QString& line : lines) {
          QString trimmed = line.trimmed();
          if (trimmed.isEmpty()) continue;
          auto match = re.match(trimmed);
          if (match.hasMatch()) {
            int mins = match.captured(1).toInt();
            int secs = match.captured(2).toInt();
            double fraction = 0.0;
            if (!match.captured(3).isEmpty()) {
              QString fracStr = match.captured(3);
              if (fracStr.length() == 2) {
                fraction = fracStr.toDouble() / 100.0;
              } else {
                fraction = fracStr.toDouble() / 1000.0;
              }
            }
            double totalTime = (mins * 60.0) + secs + fraction;
            QString text = match.captured(4).trimmed();
            if (!text.isEmpty()) {
              QVariantMap item;
              item["time"] = totalTime;
              item["text"] = text;
              syncedList.append(item);
            }
          }
        }

        if (!syncedList.isEmpty()) {
          m_syncedLyrics = syncedList;
          m_isSyncedLyrics = true;
          m_hasLyrics = true;
        } else if (!content.trimmed().isEmpty()) {
          m_lyricsText = content.trimmed();
          m_isSyncedLyrics = false;
          m_hasLyrics = true;
        }
      }
    }
  } catch (...) {}

  emit lyricsChanged();
}

void PlaybackController::next() {
  if (m_engine.queue().empty())
    return;

  if (m_engine.queue().currentIndex() + 1 >= m_engine.queue().size() &&
      m_engine.queue().repeatMode() == core::RepeatMode::Off) {
    // Wrap around to start of playlist on explicit user click
    m_engine.queue().setCurrentIndex(0);
    auto cur = m_engine.queue().currentItem();
    if (cur.has_value()) {
      m_engine.playTrack(cur->filePath);
    }
  } else {
    m_engine.nextTrack();
  }
  updateCurrentTrackMetadata();
}

void PlaybackController::previous() {
  if (m_engine.queue().empty())
    return;

  if (m_engine.currentPositionSeconds() > 3.0) {
    m_engine.seek(0.0);
  } else {
    if (m_engine.queue().currentIndex() == 0 &&
        m_engine.queue().repeatMode() == core::RepeatMode::Off) {
      m_engine.seek(0.0);
    } else {
      m_engine.previousTrack();
    }
  }
  updateCurrentTrackMetadata();
}

void PlaybackController::seek(double seconds) {
  m_engine.seek(seconds);
  emit positionChanged();
}

void PlaybackController::playFile(const QString &path) {
  std::string stdPath = path.toStdString();
  if (stdPath.empty())
    return;

  bool foundInQueue = false;
  size_t foundIdx = 0;

  // 1. Populate full queue from LibraryModel (preserving current sort & filter)
  if (m_libraryModel && m_libraryModel->rowCount() > 0) {
    auto items = m_libraryModel->toQueueItems();
    for (size_t i = 0; i < items.size(); ++i) {
      if (items[i].filePath == stdPath) {
        foundIdx = i;
        foundInQueue = true;
        break;
      }
    }
    if (foundInQueue) {
      m_engine.queue().setQueue(std::move(items), foundIdx);
    }
  }

  // 2. Fallback: check DatabaseManager tracks if not in LibraryModel
  if (!foundInQueue) {
    auto dbTracks = db::DatabaseManager::instance().getAllTracks("title ASC");
    std::vector<audio::QueueItem> items;
    items.reserve(dbTracks.size());
    for (size_t i = 0; i < dbTracks.size(); ++i) {
      audio::QueueItem it;
      it.filePath = dbTracks[i].filePath;
      it.title = dbTracks[i].title;
      it.artist = dbTracks[i].artist;
      it.album = dbTracks[i].album;
      it.durationSeconds = dbTracks[i].durationSeconds;
      if (it.filePath == stdPath) {
        foundIdx = i;
        foundInQueue = true;
      }
      items.push_back(std::move(it));
    }
    if (foundInQueue) {
      m_engine.queue().setQueue(std::move(items), foundIdx);
    }
  }

  // 3. Fallback: single item queue if external file
  if (!foundInQueue) {
    auto inspection = metadata::AudioProbe::probeFile(stdPath);
    std::string title = !inspection.metadata.title.empty()
                            ? inspection.metadata.title
                            : std::filesystem::path(stdPath).stem().string();
    std::string artist = !inspection.metadata.artist.empty()
                             ? inspection.metadata.artist
                             : "Unknown Artist";
    std::string album = !inspection.metadata.album.empty()
                            ? inspection.metadata.album
                            : "Unknown Album";

    audio::QueueItem item;
    item.filePath = stdPath;
    item.title = title;
    item.artist = artist;
    item.album = album;
    item.durationSeconds = inspection.metadata.durationSeconds;

    m_engine.queue().clear();
    m_engine.queue().append(item);
  }

  m_engine.playTrack(stdPath);
  updateCurrentTrackMetadata();
  emit queueChanged();
}

} // namespace soniq::services
