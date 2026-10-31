#pragma once

#include "audio/AudioPlaybackEngine.hpp"
#include <QObject>
#include <QString>
#include <QTimer>
#include <atomic>

namespace soniq::services {

class PlaybackController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY stateChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY stateChanged)
    Q_PROPERTY(QString trackTitle READ trackTitle NOTIFY trackChanged)
    Q_PROPERTY(QString trackArtist READ trackArtist NOTIFY trackChanged)
    Q_PROPERTY(QString trackAlbum READ trackAlbum NOTIFY trackChanged)
    Q_PROPERTY(QString trackFormat READ trackFormat NOTIFY trackChanged)
    Q_PROPERTY(QString trackFilePath READ trackFilePath NOTIFY trackChanged)
    Q_PROPERTY(QString trackArtworkPath READ trackArtworkPath NOTIFY trackChanged)
    Q_PROPERTY(double position READ position NOTIFY positionChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString positionFormatted READ positionFormatted NOTIFY positionChanged)
    Q_PROPERTY(QString durationFormatted READ durationFormatted NOTIFY durationChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY muteChanged)
    Q_PROPERTY(int repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(bool isShuffle READ isShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(QString bitPerfectStatus READ bitPerfectStatus NOTIFY bitPerfectChanged)
    Q_PROPERTY(QString bitPerfectExplanation READ bitPerfectExplanation NOTIFY bitPerfectChanged)
    Q_PROPERTY(float peakLevelL READ peakLevelL NOTIFY visualizerChanged)
    Q_PROPERTY(float peakLevelR READ peakLevelR NOTIFY visualizerChanged)
    Q_PROPERTY(QVariantList waveform READ waveform NOTIFY visualizerChanged)
    Q_PROPERTY(QVariantList queueItems READ queueItems NOTIFY queueChanged)
    Q_PROPERTY(int queueIndex READ queueIndex NOTIFY queueChanged)
    Q_PROPERTY(int queueCount READ queueCount NOTIFY queueChanged)
    Q_PROPERTY(QString lyricsText READ lyricsText NOTIFY lyricsChanged)
    Q_PROPERTY(QVariantList syncedLyrics READ syncedLyrics NOTIFY lyricsChanged)
    Q_PROPERTY(bool hasLyrics READ hasLyrics NOTIFY lyricsChanged)
    Q_PROPERTY(bool isSyncedLyrics READ isSyncedLyrics NOTIFY lyricsChanged)

public:
    explicit PlaybackController(audio::AudioPlaybackEngine& engine, QObject* parent = nullptr);
    ~PlaybackController() override;

    [[nodiscard]] bool isPlaying() const noexcept;
    [[nodiscard]] bool isPaused() const noexcept;

    [[nodiscard]] QString trackTitle() const;
    [[nodiscard]] QString trackArtist() const;
    [[nodiscard]] QString trackAlbum() const;
    [[nodiscard]] QString trackFormat() const;
    [[nodiscard]] QString trackFilePath() const;
    [[nodiscard]] QString trackArtworkPath() const;

    [[nodiscard]] double position() const noexcept;
    [[nodiscard]] double duration() const noexcept;
    [[nodiscard]] QString positionFormatted() const;
    [[nodiscard]] QString durationFormatted() const;

    [[nodiscard]] float volume() const noexcept;
    [[nodiscard]] bool isMuted() const noexcept;
    [[nodiscard]] int repeatMode() const noexcept;
    [[nodiscard]] bool isShuffle() const noexcept;

    [[nodiscard]] QString bitPerfectStatus() const;
    [[nodiscard]] QString bitPerfectExplanation() const;

    [[nodiscard]] float peakLevelL() const noexcept { return m_peakL.load(std::memory_order_relaxed); }
    [[nodiscard]] float peakLevelR() const noexcept { return m_peakR.load(std::memory_order_relaxed); }
    [[nodiscard]] QVariantList waveform() const;

    [[nodiscard]] QVariantList queueItems() const;
    [[nodiscard]] int queueIndex() const;
    [[nodiscard]] int queueCount() const;

    [[nodiscard]] QString lyricsText() const { return m_lyricsText; }
    [[nodiscard]] QVariantList syncedLyrics() const { return m_syncedLyrics; }
    [[nodiscard]] bool hasLyrics() const noexcept { return m_hasLyrics; }
    [[nodiscard]] bool isSyncedLyrics() const noexcept { return m_isSyncedLyrics; }

    void setLibraryModel(class LibraryModel* model) noexcept { m_libraryModel = model; }
    void updateCurrentTrackMetadata();

public slots:
    void play();
    void pause();
    void togglePlayPause();
    void stop();
    void next();
    void previous();
    void seek(double seconds);
    void playFile(const QString& path);
    void toggleShuffle();
    void setVolume(float vol);
    void setMuted(bool mute);
    void setRepeatMode(int mode);

    // Playback Queue actions
    void removeQueueItem(int index);
    void moveQueueItem(int fromIndex, int toIndex);
    void clearQueue();
    void playQueueIndex(int index);
    void enqueueTrack(const QString& filePath, bool playNext = false);

signals:
    void stateChanged();
    void trackChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void muteChanged();
    void repeatModeChanged();
    void shuffleChanged();
    void bitPerfectChanged();
    void visualizerChanged();
    void queueChanged();
    void lyricsChanged();

private:
    static QString formatTime(double seconds);
    void loadLyricsForTrack(const QString& filePath);

    audio::AudioPlaybackEngine& m_engine;
    class LibraryModel* m_libraryModel{nullptr};
    QTimer m_pollTimer;

    QString m_title{"No Track Loaded"};
    QString m_artist{"SONIQ Audio"};
    QString m_album{"Pure Sound"};
    QString m_format{"None"};
    QString m_filePath;
    QString m_artworkPath;

    std::atomic<float> m_peakL{0.0f};
    std::atomic<float> m_peakR{0.0f};
    mutable std::mutex m_vizMutex;
    std::vector<float> m_waveformSamples;

    QString m_lyricsText;
    QVariantList m_syncedLyrics;
    bool m_hasLyrics{false};
    bool m_isSyncedLyrics{false};
};

} // namespace soniq::services
