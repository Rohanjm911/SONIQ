#include "DspPipeline.hpp"
#include <algorithm>
#include <cmath>
#include <format>

namespace soniq::dsp {

DspPipeline::DspPipeline() {
    setSampleRate(44100.0);
}

void DspPipeline::setSampleRate(double sampleRate) {
    if (sampleRate > 0.0) {
        m_sampleRate = sampleRate;
        m_equalizer.setSampleRate(sampleRate);
        m_limiter.configure(sampleRate);
        m_crossfeed.configure(sampleRate);
    }
}

void DspPipeline::setStereoBalance(float balance) noexcept {
    m_stereoBalance = std::clamp(balance, -1.0f, 1.0f);
    // Linear pan law
    if (m_stereoBalance < 0.0f) {
        m_leftGain = 1.0f;
        m_rightGain = 1.0f + m_stereoBalance;
    } else {
        m_leftGain = 1.0f - m_stereoBalance;
        m_rightGain = 1.0f;
    }
}

void DspPipeline::setStereoWidth(float width) noexcept {
    m_stereoWidth = std::clamp(width, 0.0f, 2.0f);
}

void DspPipeline::setTrackReplayGainDb(float gainDb) noexcept {
    m_replayGainDb = gainDb;
    m_replayGainLinear = std::pow(10.0f, m_replayGainDb / 20.0f);
}

void DspPipeline::setPreampGainDb(float gainDb) noexcept {
    m_preampGainDb = gainDb;
    m_preampGainLinear = std::pow(10.0f, m_preampGainDb / 20.0f);
}

void DspPipeline::reset() noexcept {
    m_equalizer.resetFilterStates();
    m_limiter.reset();
    m_crossfeed.reset();
}

bool DspPipeline::altersSignal() const noexcept {
    if (m_masterBypass) return false;

    if (m_equalizer.isEnabled() && m_equalizer.altersSignal()) return true;
    if (m_limiter.isEnabled()) return true;
    if (m_crossfeed.isEnabled()) return true;
    if (std::abs(m_stereoBalance) > 0.005f) return true;
    if (std::abs(m_stereoWidth - 1.0f) > 0.005f) return true;
    if (m_replayGainEnabled && std::abs(m_replayGainDb) > 0.05f) return true;
    if (std::abs(m_preampGainDb) > 0.05f) return true;

    return false;
}

std::vector<std::string> DspPipeline::bitPerfectDisablingReasons() const {
    std::vector<std::string> reasons;
    if (m_masterBypass) return reasons;

    if (m_equalizer.isEnabled() && m_equalizer.altersSignal()) {
        reasons.push_back("Equalizer is active (frequency response altered)");
    }
    if (m_crossfeed.isEnabled()) {
        reasons.push_back("Binaural crossfeed is active (stereo acoustic modeling)");
    }
    if (std::abs(m_stereoBalance) > 0.005f) {
        reasons.push_back(std::format("Stereo balance altered ({:+.0f}%)", m_stereoBalance * 100.0f));
    }
    if (std::abs(m_stereoWidth - 1.0f) > 0.005f) {
        reasons.push_back(std::format("Stereo width altered ({:.1f}x)", m_stereoWidth));
    }
    if (m_replayGainEnabled && std::abs(m_replayGainDb) > 0.05f) {
        reasons.push_back(std::format("ReplayGain applied ({:+.1f} dB)", m_replayGainDb));
    }
    if (std::abs(m_preampGainDb) > 0.05f) {
        reasons.push_back(std::format("Master preamp gain active ({:+.1f} dB)", m_preampGainDb));
    }
    if (m_limiter.isEnabled()) {
        reasons.push_back("Peak limiter / clipping prevention is enabled");
    }

    return reasons;
}

void DspPipeline::process(float* interleavedData, size_t frames, uint16_t channels) noexcept {
    if (m_masterBypass || interleavedData == nullptr || frames == 0) return;

    // 1. ReplayGain & Preamp application
    float netGain = 1.0f;
    if (m_replayGainEnabled) netGain *= m_replayGainLinear;
    if (std::abs(m_preampGainLinear - 1.0f) > 1e-5f) netGain *= m_preampGainLinear;

    if (std::abs(netGain - 1.0f) > 1e-5f) {
        const size_t total = frames * channels;
        for (size_t i = 0; i < total; ++i) {
            interleavedData[i] *= netGain;
        }
    }

    // 2. Equalizer
    if (m_equalizer.isEnabled()) {
        m_equalizer.process(interleavedData, frames, channels);
    }

    // 3. Stereo manipulation (Balance & Width)
    if (channels == 2) {
        const bool applyBalance = (std::abs(m_stereoBalance) > 1e-4f);
        const bool applyWidth = (std::abs(m_stereoWidth - 1.0f) > 1e-4f);

        if (applyBalance || applyWidth) {
            for (size_t f = 0; f < frames; ++f) {
                float l = interleavedData[f * 2 + 0];
                float r = interleavedData[f * 2 + 1];

                if (applyWidth) {
                    float mid = 0.5f * (l + r);
                    float side = 0.5f * (l - r) * m_stereoWidth;
                    l = mid + side;
                    r = mid - side;
                }

                if (applyBalance) {
                    l *= m_leftGain;
                    r *= m_rightGain;
                }

                interleavedData[f * 2 + 0] = l;
                interleavedData[f * 2 + 1] = r;
            }
        }

        // 4. Crossfeed (headphone acoustic modeling)
        if (m_crossfeed.isEnabled()) {
            m_crossfeed.process(interleavedData, frames);
        }
    }

    // 5. Limiter (Clipping prevention at output stage)
    if (m_limiter.isEnabled()) {
        m_limiter.process(interleavedData, frames, channels);
    }
}

} // namespace soniq::dsp
