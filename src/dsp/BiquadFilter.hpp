#pragma once

#include <cmath>
#include <numbers>
#include <cstddef>
#include <complex>

namespace soniq::dsp {

enum class FilterType {
    Bypass = 0,
    LowPass,
    HighPass,
    BandPass,
    Notch,
    PeakingEQ,
    LowShelf,
    HighShelf
};

class BiquadFilter {
public:
    BiquadFilter() = default;

    void configure(FilterType type, double sampleRate, double frequency, double Q, double gainDb = 0.0);

    void reset() noexcept;

    [[nodiscard]] float process(float in) noexcept {
        if (m_type == FilterType::Bypass) return in;
        // Direct Form II Transposed
        double inD = static_cast<double>(in);
        double outD = m_b0 * inD + m_z1;
        m_z1 = m_b1 * inD - m_a1 * outD + m_z2;
        m_z2 = m_b2 * inD - m_a2 * outD;
        return static_cast<float>(outD);
    }

    void processInterleaved(float* data, size_t frames, uint16_t channels, uint16_t targetChannel) noexcept {
        if (m_type == FilterType::Bypass) return;
        for (size_t f = 0; f < frames; ++f) {
            size_t idx = f * channels + targetChannel;
            data[idx] = process(data[idx]);
        }
    }

    [[nodiscard]] double magnitudeResponse(double freq, double sampleRate) const noexcept;

    [[nodiscard]] FilterType type() const noexcept { return m_type; }
    [[nodiscard]] double frequency() const noexcept { return m_frequency; }
    [[nodiscard]] double Q() const noexcept { return m_Q; }
    [[nodiscard]] double gainDb() const noexcept { return m_gainDb; }

private:
    FilterType m_type{FilterType::Bypass};
    double m_frequency{1000.0};
    double m_Q{0.7071};
    double m_gainDb{0.0};

    // Normalized coefficients (a0 normalized to 1.0)
    double m_b0{1.0};
    double m_b1{0.0};
    double m_b2{0.0};
    double m_a1{0.0};
    double m_a2{0.0};

    // Direct Form II Transposed state registers
    double m_z1{0.0};
    double m_z2{0.0};
};

} // namespace soniq::dsp
