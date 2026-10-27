#include "Equalizer.hpp"
#include <cmath>
#include <algorithm>

namespace soniq::dsp {

const std::array<double, Equalizer::kNum10Bands>& Equalizer::frequencies10Band() {
    static const std::array<double, kNum10Bands> freqs = {
        31.25, 62.5, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0
    };
    return freqs;
}

const std::array<double, Equalizer::kNum31Bands>& Equalizer::frequencies31Band() {
    static const std::array<double, kNum31Bands> freqs = {
        20.0, 25.0, 31.5, 40.0, 50.0, 63.0, 80.0, 100.0, 125.0, 160.0,
        200.0, 250.0, 315.0, 400.0, 500.0, 630.0, 800.0, 1000.0, 1250.0, 1600.0,
        2000.0, 2500.0, 3150.0, 4000.0, 5000.0, 6300.0, 8000.0, 10000.0, 12500.0, 16000.0,
        20000.0
    };
    return freqs;
}

Equalizer::Equalizer() {
    m_gains10.fill(0.0);
    m_gains31.fill(0.0);
    rebuildFilters();
}

void Equalizer::setSampleRate(double sampleRate) {
    if (sampleRate > 0.0 && std::abs(m_sampleRate - sampleRate) > 1.0) {
        m_sampleRate = sampleRate;
        rebuildFilters();
    }
}

void Equalizer::setMode(EqualizerMode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        rebuildFilters();
    }
}

void Equalizer::setPreampDb(double gainDb) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_preampDb = std::clamp(gainDb, -24.0, 24.0);
    m_preampLinear = static_cast<float>(std::pow(10.0, m_preampDb / 20.0));
}

void Equalizer::setBandGain10(size_t band, double gainDb) {
    if (band < kNum10Bands) {
        m_gains10[band] = std::clamp(gainDb, -20.0, 20.0);
        if (m_mode == EqualizerMode::TenBand) {
            rebuildFilters();
        }
    }
}

double Equalizer::bandGain10(size_t band) const noexcept {
    return (band < kNum10Bands) ? m_gains10[band] : 0.0;
}

void Equalizer::setBandGain31(size_t band, double gainDb) {
    if (band < kNum31Bands) {
        m_gains31[band] = std::clamp(gainDb, -20.0, 20.0);
        if (m_mode == EqualizerMode::ThirtyOneBand) {
            rebuildFilters();
        }
    }
}

double Equalizer::bandGain31(size_t band) const noexcept {
    return (band < kNum31Bands) ? m_gains31[band] : 0.0;
}

void Equalizer::setParametricBands(const std::vector<ParametricBand>& bands) {
    m_parametricBands = bands;
    if (m_mode == EqualizerMode::Parametric) {
        rebuildFilters();
    }
}

void Equalizer::resetGains() noexcept {
    m_preampDb = 0.0;
    m_preampLinear = 1.0f;
    m_gains10.fill(0.0);
    m_gains31.fill(0.0);
    m_parametricBands.clear();
    rebuildFilters();
}

void Equalizer::resetFilterStates() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& chFilters : m_filters) {
        for (auto& f : chFilters) {
            f.reset();
        }
    }
}

bool Equalizer::altersSignal() const noexcept {
    if (!m_enabled) return false;
    if (std::abs(m_preampDb) > 0.01) return true;

    if (m_mode == EqualizerMode::TenBand) {
        for (double g : m_gains10) {
            if (std::abs(g) > 0.01) return true;
        }
    } else if (m_mode == EqualizerMode::ThirtyOneBand) {
        for (double g : m_gains31) {
            if (std::abs(g) > 0.01) return true;
        }
    } else if (m_mode == EqualizerMode::Parametric) {
        for (const auto& b : m_parametricBands) {
            if (b.enabled && std::abs(b.gainDb) > 0.01) return true;
        }
    }
    return false;
}

void Equalizer::rebuildFilters() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t numBands = 0;
    if (m_mode == EqualizerMode::TenBand) {
        numBands = kNum10Bands;
    } else if (m_mode == EqualizerMode::ThirtyOneBand) {
        numBands = kNum31Bands;
    } else if (m_mode == EqualizerMode::Parametric) {
        numBands = m_parametricBands.size();
    }

    for (size_t ch = 0; ch < kMaxChannels; ++ch) {
        m_filters[ch].resize(numBands);
    }

    if (m_mode == EqualizerMode::TenBand) {
        const auto& freqs = frequencies10Band();
        // Q ~ 1.414 for 1-octave bandwidth
        constexpr double kQ = 1.4142;

        for (size_t i = 0; i < kNum10Bands; ++i) {
            FilterType type = FilterType::PeakingEQ;
            if (i == 0) type = FilterType::LowShelf;
            else if (i == kNum10Bands - 1) type = FilterType::HighShelf;

            for (size_t ch = 0; ch < kMaxChannels; ++ch) {
                m_filters[ch][i].configure(type, m_sampleRate, freqs[i], kQ, m_gains10[i]);
            }
        }
    } else if (m_mode == EqualizerMode::ThirtyOneBand) {
        const auto& freqs = frequencies31Band();
        // Q ~ 4.318 for 1/3-octave bandwidth
        constexpr double kQ = 4.318;

        for (size_t i = 0; i < kNum31Bands; ++i) {
            FilterType type = FilterType::PeakingEQ;
            if (i == 0) type = FilterType::LowShelf;
            else if (i == kNum31Bands - 1) type = FilterType::HighShelf;

            for (size_t ch = 0; ch < kMaxChannels; ++ch) {
                m_filters[ch][i].configure(type, m_sampleRate, freqs[i], kQ, m_gains31[i]);
            }
        }
    } else if (m_mode == EqualizerMode::Parametric) {
        for (size_t i = 0; i < m_parametricBands.size(); ++i) {
            const auto& b = m_parametricBands[i];
            FilterType type = b.enabled ? b.type : FilterType::Bypass;

            for (size_t ch = 0; ch < kMaxChannels; ++ch) {
                m_filters[ch][i].configure(type, m_sampleRate, b.frequency, b.Q, b.gainDb);
            }
        }
    }
}

void Equalizer::process(float* interleavedData, size_t frames, uint16_t channels) noexcept {
    if (!m_enabled || !altersSignal() || interleavedData == nullptr || frames == 0) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    const uint16_t activeChannels = std::min(channels, static_cast<uint16_t>(kMaxChannels));

    // 1. Preamp application
    if (std::abs(m_preampLinear - 1.0f) > 1e-5f) {
        for (size_t i = 0; i < frames * channels; ++i) {
            interleavedData[i] *= m_preampLinear;
        }
    }

    // 2. Cascade filter processing per channel
    for (uint16_t ch = 0; ch < activeChannels; ++ch) {
        auto& filters = m_filters[ch];
        for (auto& filter : filters) {
            filter.processInterleaved(interleavedData, frames, channels, ch);
        }
    }
}

double Equalizer::gainAtFrequency(double freq) const noexcept {
    if (!m_enabled) return 0.0;

    std::lock_guard<std::mutex> lock(m_mutex);
    double netGain = m_preampDb;
    if (m_filters[0].empty()) return netGain;

    double totalMag = 1.0;
    for (const auto& f : m_filters[0]) {
        totalMag *= f.magnitudeResponse(freq, m_sampleRate);
    }

    if (totalMag > 1e-12) {
        netGain += 20.0 * std::log10(totalMag);
    }
    return netGain;
}

std::vector<std::string> Equalizer::presetNames() {
    return {
        "Flat",
        "Acoustic",
        "Bass Booster",
        "Bass Reducer",
        "Classical",
        "Electronic",
        "Hip-Hop",
        "Jazz",
        "Pop",
        "Rock",
        "Vocal Booster"
    };
}

void Equalizer::applyPreset(const std::string& name) {
    m_mode = EqualizerMode::TenBand;
    m_preampDb = 0.0;
    m_preampLinear = 1.0f;

    if (name == "Flat") {
        m_gains10 = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    } else if (name == "Acoustic") {
        m_gains10 = {4.5, 4.0, 3.0, 1.0, 1.5, 1.5, 2.5, 3.5, 3.0, 1.5};
    } else if (name == "Bass Booster") {
        m_gains10 = {6.0, 5.0, 4.0, 2.5, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    } else if (name == "Bass Reducer") {
        m_gains10 = {-6.0, -5.0, -4.0, -2.5, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    } else if (name == "Classical") {
        m_gains10 = {5.0, 4.0, 3.0, 2.5, -1.5, -1.5, 0.0, 2.0, 3.5, 4.0};
    } else if (name == "Electronic") {
        m_gains10 = {4.5, 4.0, 1.5, 0.0, -2.0, 2.0, 1.0, 1.5, 4.0, 4.5};
    } else if (name == "Hip-Hop") {
        m_gains10 = {5.5, 4.5, 2.0, 3.0, -1.0, -1.5, 1.5, -1.0, 2.0, 3.0};
    } else if (name == "Jazz") {
        m_gains10 = {4.0, 3.0, 1.5, 2.0, -1.5, -1.5, 0.0, 1.5, 3.0, 3.5};
    } else if (name == "Pop") {
        m_gains10 = {-1.5, -1.0, 0.0, 2.0, 4.0, 4.0, 2.0, 0.0, -1.0, -1.5};
    } else if (name == "Rock") {
        m_gains10 = {5.0, 4.0, 3.0, 1.5, -0.5, -1.0, 0.5, 2.5, 3.5, 4.5};
    } else if (name == "Vocal Booster") {
        m_gains10 = {-1.0, -2.0, -2.0, 1.0, 3.5, 3.5, 3.0, 1.5, 0.0, -1.5};
    }

    rebuildFilters();
}

} // namespace soniq::dsp
