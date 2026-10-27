#include "DspService.hpp"

namespace soniq::services {

DspService::DspService(audio::AudioPlaybackEngine& engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine) {}

bool DspService::isEnabled() const noexcept {
    return m_engine.dsp().equalizer().isEnabled();
}

void DspService::setEnabled(bool enable) {
    if (m_engine.dsp().equalizer().isEnabled() != enable) {
        m_engine.dsp().equalizer().setEnabled(enable);
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
        emit dspChanged();
    }
}

double DspService::preampDb() const noexcept {
    return m_engine.dsp().equalizer().preampDb();
}

void DspService::setPreampDb(double db) {
    m_engine.dsp().equalizer().setPreampDb(db);
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

void DspService::setBandGain10(int band, double gainDb) {
    m_engine.dsp().equalizer().setBandGain10(static_cast<size_t>(band), gainDb);
    emit dspChanged();
}

void DspService::setBandGain31(int band, double gainDb) {
    m_engine.dsp().equalizer().setBandGain31(static_cast<size_t>(band), gainDb);
    emit dspChanged();
}

void DspService::applyPreset(const QString& name) {
    m_engine.dsp().equalizer().applyPreset(name.toStdString());
    emit dspChanged();
}

void DspService::resetEq() {
    m_engine.dsp().equalizer().resetGains();
    emit dspChanged();
}

} // namespace soniq::services
