#pragma once

#include "database/DatabaseManager.hpp"
#include <QObject>
#include <QString>
#include <QStringList>

namespace soniq::services {

class SettingsService : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY settingsChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY settingsChanged)
    Q_PROPERTY(bool exclusiveMode READ exclusiveMode WRITE setExclusiveMode NOTIFY settingsChanged)
    Q_PROPERTY(int bufferSize READ bufferSize WRITE setBufferSize NOTIFY settingsChanged)
    Q_PROPERTY(bool replayGainEnabled READ replayGainEnabled WRITE setReplayGainEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool crossfadeEnabled READ crossfadeEnabled WRITE setCrossfadeEnabled NOTIFY settingsChanged)
    Q_PROPERTY(double crossfadeSeconds READ crossfadeSeconds WRITE setCrossfadeSeconds NOTIFY settingsChanged)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY settingsChanged)

public:
    explicit SettingsService(QObject* parent = nullptr);

    [[nodiscard]] QString themeMode() const;
    void setThemeMode(const QString& mode);

    [[nodiscard]] QString accentColor() const;
    void setAccentColor(const QString& color);

    [[nodiscard]] bool exclusiveMode() const;
    void setExclusiveMode(bool enabled);

    [[nodiscard]] int bufferSize() const;
    void setBufferSize(int size);

    [[nodiscard]] bool replayGainEnabled() const;
    void setReplayGainEnabled(bool enabled);

    [[nodiscard]] bool crossfadeEnabled() const;
    void setCrossfadeEnabled(bool enabled);

    [[nodiscard]] double crossfadeSeconds() const;
    void setCrossfadeSeconds(double sec);

    [[nodiscard]] bool reducedMotion() const;
    void setReducedMotion(bool enabled);

public slots:
    void resetToDefaults();

signals:
    void settingsChanged();
};

} // namespace soniq::services
