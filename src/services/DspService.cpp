#include "DspService.hpp"
#include "database/DatabaseManager.hpp"
#include <sstream>
#include <iomanip>

namespace soniq::services {

DspService::DspService(audio::AudioPlaybackEngine& engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine) {
    loadSettings();
}

void DspService::loadSettings() {
    auto& db = db::DatabaseManager::instance();
    bool enabled = (db.getPreference("dsp_enabled", "0") == "1");
    m_engine.dsp().equalizer().setEnabled(enabled);

    m_currentPreset = QString::fromStdString(db.getPreference("dsp_preset", "Flat"));
    if (m_currentPreset.isEmpty()) m_currentPreset = "Flat";

    std::string preampStr = db.getPreference("dsp_preamp", "0.0");
    try {
        double preamp = std::stod(preampStr);
        m_engine.dsp().equalizer().setPreampDb(preamp);
    } catch (...) {}

    std::string gainsStr = db.getPreference("dsp_gains10", "");
    if (!gainsStr.empty()) {
        std::stringstream ss(gainsStr);
        std::string item;
        size_t b = 0;
        while (std::getline(ss, item, ',') && b < dsp::Equalizer::kNum10Bands) {
            try {
                double g = std::stod(item);
                m_engine.dsp().equalizer().setBandGain10(b, g);
            } catch (...) {}
            b++;
        }
    } else if (m_currentPreset != "Flat" && m_currentPreset != "Custom") {
        m_engine.dsp().equalizer().applyPreset(m_currentPreset.toStdString());
    }

    bool cf = (db.getPreference("dsp_crossfeed", "0") == "1");
    m_engine.dsp().crossfeed().setEnabled(cf);

    bool lim = (db.getPreference("dsp_limiter", "1") != "0");
    m_engine.dsp().limiter().setEnabled(lim);
}

void DspService::saveSettings() {
    auto& db = db::DatabaseManager::instance();
    db.setPreference("dsp_enabled", m_engine.dsp().equalizer().isEnabled() ? "1" : "0");
    db.setPreference("dsp_preset", m_currentPreset.toStdString());
    db.setPreference("dsp_preamp", std::to_string(m_engine.dsp().equalizer().preampDb()));

    std::stringstream ss;
    const auto& g = m_engine.dsp().equalizer().gains10();
    for (size_t i = 0; i < g.size(); ++i) {
        if (i > 0) ss << ",";
        ss << std::fixed << std::setprecision(2) << g[i];
    }
    db.setPreference("dsp_gains10", ss.str());

    db.setPreference("dsp_crossfeed", m_engine.dsp().crossfeed().isEnabled() ? "1" : "0");
    db.setPreference("dsp_limiter", m_engine.dsp().limiter().isEnabled() ? "1" : "0");
}

bool DspService::isEnabled() const noexcept {
    return m_engine.dsp().equalizer().isEnabled();
}

void DspService::setEnabled(bool enable) {
    if (m_engine.dsp().equalizer().isEnabled() != enable) {
        m_engine.dsp().equalizer().setEnabled(enable);
        saveSettings();
        emit dspChanged();
    }
}

int DspService::mode() const noexcept {
    return static_cast<int>(m_engine.dsp().equalizer().mode());
}

void DspService::setMode(int m) {
    auto eqMode = static_cast<dsp::EqualizerMode>(m);
    if (m_engine.dsp().equalizer().mode() != eqMode) {
        m_engine.dsp().equalizer().setMode(eqMode);
        saveSettings();
        emit dspChanged();
    }
}

double DspService::preampDb() const noexcept {
    return m_engine.dsp().equalizer().preampDb();
}

void DspService::setPreampDb(double db) {
    if (!m_engine.dsp().equalizer().isEnabled()) {
        m_engine.dsp().equalizer().setEnabled(true);
    }
    m_engine.dsp().equalizer().setPreampDb(db);
    saveSettings();
    emit dspChanged();
}

QVariantList DspService::gains10() const {
    QVariantList list;
    const auto& g = m_engine.dsp().equalizer().gains10();
    for (double val : g) {
        list.append(val);
    }
    return list;
}

QVariantList DspService::gains31() const {
    QVariantList list;
    const auto& g = m_engine.dsp().equalizer().gains31();
    for (double val : g) {
        list.append(val);
    }
    return list;
}

bool DspService::crossfeedEnabled() const noexcept {
    return m_engine.dsp().crossfeed().isEnabled();
}

void DspService::setCrossfeedEnabled(bool enable) {
    if (m_engine.dsp().crossfeed().isEnabled() != enable) {
        m_engine.dsp().crossfeed().setEnabled(enable);
        saveSettings();
        emit dspChanged();
    }
}

float DspService::stereoBalance() const noexcept {
    return m_engine.dsp().stereoBalance();
}

void DspService::setStereoBalance(float balance) {
    m_engine.dsp().setStereoBalance(balance);
    emit dspChanged();
}

float DspService::stereoWidth() const noexcept {
    return m_engine.dsp().stereoWidth();
}

void DspService::setStereoWidth(float width) {
    m_engine.dsp().setStereoWidth(width);
    emit dspChanged();
}

bool DspService::limiterEnabled() const noexcept {
    return m_engine.dsp().limiter().isEnabled();
}

void DspService::setLimiterEnabled(bool enable) {
    if (m_engine.dsp().limiter().isEnabled() != enable) {
        m_engine.dsp().limiter().setEnabled(enable);
        saveSettings();
        emit dspChanged();
    }
}

QStringList DspService::presets() const {
    QStringList list;
    for (const auto& name : dsp::Equalizer::presetNames()) {
        list.append(QString::fromStdString(name));
    }
    return list;
}

bool DspService::altersSignal() const noexcept {
    return m_engine.dsp().altersSignal();
}

QStringList DspService::disablingReasons() const {
    QStringList list;
    for (const auto& reason : m_engine.dsp().bitPerfectDisablingReasons()) {
        list.append(QString::fromStdString(reason));
    }
    return list;
}

double DspService::gainAtFrequency(double freq) const {
    return m_engine.dsp().equalizer().gainAtFrequency(freq);
}

void DspService::setBandGain10(int band, double gainDb) {
    if (!m_engine.dsp().equalizer().isEnabled()) {
        m_engine.dsp().equalizer().setEnabled(true);
    }
    m_currentPreset = "Custom";
    m_engine.dsp().equalizer().setBandGain10(static_cast<size_t>(band), gainDb);
    saveSettings();
    emit dspChanged();
}

void DspService::setBandGain31(int band, double gainDb) {
    if (!m_engine.dsp().equalizer().isEnabled()) {
        m_engine.dsp().equalizer().setEnabled(true);
    }
    m_currentPreset = "Custom";
    m_engine.dsp().equalizer().setBandGain31(static_cast<size_t>(band), gainDb);
    saveSettings();
    emit dspChanged();
}

void DspService::applyPreset(const QString& name) {
    m_currentPreset = name;
    if (name != "Flat") {
        m_engine.dsp().equalizer().setEnabled(true);
    }
    m_engine.dsp().equalizer().applyPreset(name.toStdString());
    saveSettings();
    emit dspChanged();
}

void DspService::resetEq() {
    m_currentPreset = "Flat";
    m_engine.dsp().equalizer().resetGains();
    saveSettings();
    emit dspChanged();
}

} // namespace soniq::services
