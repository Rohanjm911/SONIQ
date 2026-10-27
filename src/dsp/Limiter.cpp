#include "Limiter.hpp"
#include <algorithm>

namespace soniq::dsp {

Limiter::Limiter() {
    configure(44100.0, -0.3, 50.0);
}

void Limiter::configure(double sampleRate, double thresholdDb, double releaseTimeMs) {
    m_sampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    m_releaseTimeMs = std::clamp(releaseTimeMs, 1.0, 1000.0);

    setThresholdDb(thresholdDb);
    setCeilingDb(-0.1);

    // Exponential release factor per sample: exp(-1 / (sampleRate * releaseTimeSec))
    double releaseSamples = (m_releaseTimeMs / 1000.0) * m_sampleRate;
    m_releaseCoeff = static_cast<float>(std::exp(-1.0 / releaseSamples));
    m_currentGain = 1.0f;
    m_triggered = false;
}

void Limiter::setThresholdDb(double db) {
    m_thresholdDb = std::clamp(db, -12.0, 0.0);
    m_thresholdLinear = static_cast<float>(std::pow(10.0, m_thresholdDb / 20.0));
}

void Limiter::setCeilingDb(double db) {
    m_ceilingDb = std::clamp(db, -6.0, 0.0);
    m_ceilingLinear = static_cast<float>(std::pow(10.0, m_ceilingDb / 20.0));
}

void Limiter::reset() noexcept {
    m_currentGain = 1.0f;
    m_triggered = false;
}

float Limiter::currentGainReductionDb() const noexcept {
    if (m_currentGain >= 0.999f) return 0.0f;
    return static_cast<float>(20.0 * std::log10(std::max(1e-4f, m_currentGain)));
}

void Limiter::process(float* interleavedData, size_t frames, uint16_t channels) noexcept {
    if (!m_enabled || interleavedData == nullptr || frames == 0) return;

    m_triggered = false;

    for (size_t f = 0; f < frames; ++f) {
        // Find peak across all channels for this frame
        float framePeak = 0.0f;
        for (uint16_t c = 0; c < channels; ++c) {
            float absSample = std::abs(interleavedData[f * channels + c]);
            if (absSample > framePeak) framePeak = absSample;
        }

        // Calculate target gain
        float targetGain = 1.0f;
        if (framePeak > m_thresholdLinear) {
            // Soft-knee limiting
            float overshoot = framePeak - m_thresholdLinear;
            float compressedPeak = m_thresholdLinear + overshoot / (1.0f + overshoot / (m_ceilingLinear - m_thresholdLinear));
            targetGain = compressedPeak / framePeak;
            m_triggered = true;
        }

        // Fast attack, smooth exponential release
        if (targetGain < m_currentGain) {
            m_currentGain = targetGain; // instantaneous peak catching
        } else {
            m_currentGain = targetGain + m_releaseCoeff * (m_currentGain - targetGain);
        }

        // Apply gain and hard-clamp at absolute ceiling to guarantee zero clipping
        for (uint16_t c = 0; c < channels; ++c) {
            float val = interleavedData[f * channels + c] * m_currentGain;
            interleavedData[f * channels + c] = std::clamp(val, -m_ceilingLinear, m_ceilingLinear);
        }
    }
}

} // namespace soniq::dsp
