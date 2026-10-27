#pragma once

#include "audio/AudioPlaybackEngine.hpp"
#include <QObject>
#include <QString>
#include <QTimer>

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
    void setVolume(float vol);

    [[nodiscard]] bool isMuted() const noexcept;
    void setMuted(bool mute);

    [[nodiscard]] int repeatMode() const noexcept;
    void setRepeatMode(int mode);

    [[nodiscard]] bool isShuffle() const noexcept;

    [[nodiscard]] QString bitPerfectStatus() const;
    [[nodiscard]] QString bitPerfectExplanation() const;

    [[nodiscard]] float peakLevelL() const noexcept { return m_peakL; }
    [[nodiscard]] float peakLevelR() const noexcept { return m_peakR; }

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

private:
    static QString formatTime(double seconds);

    audio::AudioPlaybackEngine& m_engine;
    QTimer m_pollTimer;

    QString m_title{"No Track Loaded"};
    QString m_artist{"SONIQ Audio"};
    QString m_album{"Pure Sound"};
    QString m_format{"None"};
    QString m_filePath;
    QString m_artworkPath;

    float m_peakL{0.0f};
    float m_peakR{0.0f};
};

} // namespace soniq::services
