#include "PlaybackController.hpp"
#include "metadata/AudioProbe.hpp"
#include "metadata/ArtworkExtractor.hpp"
#include "database/DatabaseManager.hpp"
#include <cmath>
#include <filesystem>

namespace soniq::services {

PlaybackController::PlaybackController(audio::AudioPlaybackEngine& engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine) {

    m_engine.setStateChangedCallback([this](core::PlaybackState) {
        QMetaObject::invokeMethod(this, [this]() {
            emit stateChanged();
            emit bitPerfectChanged();
        }, Qt::QueuedConnection);
    });

    m_engine.setVisualizerCallback([this](const audio::EngineVisualizerData& viz) {
        m_peakL.store(viz.peakLevelL, std::memory_order_relaxed);
        m_peakR.store(viz.peakLevelR, std::memory_order_relaxed);
    });

    m_engine.setTrackEndedCallback([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            auto cur = m_engine.queue().currentItem();
            if (cur.has_value()) {
                m_title = QString::fromStdString(cur->title);
                m_artist = QString::fromStdString(cur->artist);
                m_album = QString::fromStdString(cur->album);
                m_filePath = QString::fromStdString(cur->filePath);
                m_artworkPath = QString::fromStdString(metadata::ArtworkExtractor::instance().getOrExtractArtwork(cur->filePath));
                emit trackChanged();
                emit durationChanged();
                emit bitPerfectChanged();
            }
        }, Qt::QueuedConnection);
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

QString PlaybackController::trackTitle() const {
    return m_title;
}

QString PlaybackController::trackArtist() const {
    return m_artist;
}

QString PlaybackController::trackAlbum() const {
    return m_album;
}

QString PlaybackController::trackFormat() const {
    return m_format;
}

QString PlaybackController::trackFilePath() const {
    return m_filePath;
}

QString PlaybackController::trackArtworkPath() const {
    return m_artworkPath;
}

double PlaybackController::position() const noexcept {
    return m_engine.currentPositionSeconds();
}

double PlaybackController::duration() const noexcept {
    return m_engine.totalDurationSeconds();
}

QString PlaybackController::formatTime(double seconds) {
    if (seconds <= 0.0) return "0:00";
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

float PlaybackController::volume() const noexcept {
    return m_engine.volume();
}

void PlaybackController::setVolume(float vol) {
    if (std::abs(m_engine.volume() - vol) > 1e-4f) {
        m_engine.setVolume(vol);
        emit volumeChanged();
        emit bitPerfectChanged();
    }
}

bool PlaybackController::isMuted() const noexcept {
    return m_engine.isMuted();
}

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
        case core::BitPerfectStatus::Verified:   return "Verified";
        case core::BitPerfectStatus::Available:  return "Available";
        case core::BitPerfectStatus::Unavailable:return "Unavailable";
        case core::BitPerfectStatus::Unknown:    return "Unknown";
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

void PlaybackController::next() {
    m_engine.nextTrack();
    auto cur = m_engine.queue().currentItem();
    if (cur.has_value()) {
        m_title = QString::fromStdString(cur->title);
        m_artist = QString::fromStdString(cur->artist);
        m_album = QString::fromStdString(cur->album);
        m_filePath = QString::fromStdString(cur->filePath);
        m_artworkPath = QString::fromStdString(metadata::ArtworkExtractor::instance().getOrExtractArtwork(cur->filePath));
        emit trackChanged();
        emit durationChanged();
    }
}

void PlaybackController::previous() {
    m_engine.previousTrack();
    auto cur = m_engine.queue().currentItem();
    if (cur.has_value()) {
        m_title = QString::fromStdString(cur->title);
        m_artist = QString::fromStdString(cur->artist);
        m_album = QString::fromStdString(cur->album);
        m_filePath = QString::fromStdString(cur->filePath);
        m_artworkPath = QString::fromStdString(metadata::ArtworkExtractor::instance().getOrExtractArtwork(cur->filePath));
        emit trackChanged();
        emit durationChanged();
    }
}

void PlaybackController::seek(double seconds) {
    m_engine.seek(seconds);
    emit positionChanged();
}

void PlaybackController::playFile(const QString& path) {
    std::string stdPath = path.toStdString();
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

    m_title = QString::fromStdString(title);
    m_artist = QString::fromStdString(artist);
    m_album = QString::fromStdString(album);
    m_format = QString::fromStdString(inspection.format.toDetailedString());
    m_filePath = path;
    m_artworkPath = QString::fromStdString(metadata::ArtworkExtractor::instance().getOrExtractArtwork(stdPath));

    audio::QueueItem item;
    item.filePath = stdPath;
    item.title = title;
    item.artist = artist;
    item.album = album;
    item.durationSeconds = inspection.metadata.durationSeconds;

    m_engine.queue().clear();
    m_engine.queue().append(item);

    m_engine.playTrack(stdPath);

    emit trackChanged();
    emit durationChanged();
    emit stateChanged();
    emit bitPerfectChanged();
}

} // namespace soniq::services
