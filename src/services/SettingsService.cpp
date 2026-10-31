#include "SettingsService.hpp"

namespace soniq::services {

SettingsService::SettingsService(QObject* parent)
    : QObject(parent) {}

QString SettingsService::themeMode() const {
    auto val = db::DatabaseManager::instance().getPreference("theme_mode", "Dark");
    return QString::fromStdString(val);
}

void SettingsService::setThemeMode(const QString& mode) {
    db::DatabaseManager::instance().setPreference("theme_mode", mode.toStdString());
    emit settingsChanged();
}

QString SettingsService::accentColor() const {
    auto val = db::DatabaseManager::instance().getPreference("accent_color", "#FA2D48"); // Apple Music Crimson / Vibrant Coral
    return QString::fromStdString(val);
}

void SettingsService::setAccentColor(const QString& color) {
    db::DatabaseManager::instance().setPreference("accent_color", color.toStdString());
    emit settingsChanged();
}

bool SettingsService::exclusiveMode() const {
    auto val = db::DatabaseManager::instance().getPreference("exclusive_mode", "false");
    return val == "true";
}

void SettingsService::setExclusiveMode(bool enabled) {
    db::DatabaseManager::instance().setPreference("exclusive_mode", enabled ? "true" : "false");
    emit settingsChanged();
}

int SettingsService::bufferSize() const {
    auto val = db::DatabaseManager::instance().getPreference("buffer_size", "1024");
    try {
        return std::stoi(val);
    } catch (...) {
        return 1024;
    }
}

void SettingsService::setBufferSize(int size) {
    db::DatabaseManager::instance().setPreference("buffer_size", std::to_string(size));
    emit settingsChanged();
}

bool SettingsService::replayGainEnabled() const {
    auto val = db::DatabaseManager::instance().getPreference("replaygain_enabled", "false");
    return val == "true";
}

void SettingsService::setReplayGainEnabled(bool enabled) {
    db::DatabaseManager::instance().setPreference("replaygain_enabled", enabled ? "true" : "false");
    emit settingsChanged();
}

bool SettingsService::crossfadeEnabled() const {
    auto val = db::DatabaseManager::instance().getPreference("crossfade_enabled", "false");
    return val == "true";
}

void SettingsService::setCrossfadeEnabled(bool enabled) {
    db::DatabaseManager::instance().setPreference("crossfade_enabled", enabled ? "true" : "false");
    emit settingsChanged();
}

double SettingsService::crossfadeSeconds() const {
    auto val = db::DatabaseManager::instance().getPreference("crossfade_seconds", "2.0");
    try {
        return std::stod(val);
    } catch (...) {
        return 2.0;
    }
}

void SettingsService::setCrossfadeSeconds(double sec) {
    db::DatabaseManager::instance().setPreference("crossfade_seconds", std::to_string(sec));
    emit settingsChanged();
}

bool SettingsService::reducedMotion() const {
    auto val = db::DatabaseManager::instance().getPreference("reduced_motion", "false");
    return val == "true";
}

void SettingsService::setReducedMotion(bool enabled) {
    db::DatabaseManager::instance().setPreference("reduced_motion", enabled ? "true" : "false");
    emit settingsChanged();
}

void SettingsService::resetToDefaults() {
    setThemeMode("Dark");
    setAccentColor("#FA2D48");
    setExclusiveMode(false);
    setBufferSize(1024);
    setReplayGainEnabled(false);
    setCrossfadeEnabled(false);
    setCrossfadeSeconds(2.0);
    setReducedMotion(false);
}

} // namespace soniq::services
