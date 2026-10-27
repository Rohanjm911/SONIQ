#pragma once

#include <cstddef>
#include <cstdint>

namespace soniq::dsp {

enum class CrossfeedPreset {
    Default = 0, // 700 Hz, 4.5 dB
    Subtle,      // 700 Hz, 6.0 dB
    Strong,      // 650 Hz, 3.0 dB
    Custom
};

class Crossfeed {
public:
    Crossfeed();

    void configure(double sampleRate, double fcut = 700.0, double feed = 4.5);
    void setPreset(CrossfeedPreset preset);

    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }

    void reset() noexcept;

    void process(float* interleavedStereo, size_t frames) noexcept;

private:
    bool m_enabled{false};
    double m_sampleRate{44100.0};
    double m_fcut{700.0}; // Cutoff frequency in Hz
    double m_feed{4.5};   // Feed level in dB

    // Filter coefficients for crossfeed IIR
    double m_a0_l{1.0}, m_b1_l{0.0};
    double m_a0_r{1.0}, m_b1_r{0.0};
    double m_g_l{0.0}, m_g_r{0.0};

    // States
    double m_z_l{0.0};
    double m_z_r{0.0};
};

} // namespace soniq::dsp
