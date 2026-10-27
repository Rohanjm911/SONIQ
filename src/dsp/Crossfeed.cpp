#include "Crossfeed.hpp"
#include <cmath>
#include <numbers>
#include <algorithm>

namespace soniq::dsp {

Crossfeed::Crossfeed() {
    setPreset(CrossfeedPreset::Default);
}

void Crossfeed::configure(double sampleRate, double fcut, double feed) {
    m_sampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    m_fcut = std::clamp(fcut, 300.0, 2000.0);
    m_feed = std::clamp(feed, 1.0, 15.0);

    // Bauer stereophonic-to-binaural DSP algorithm
    const double fc = m_fcut;
    const double g = std::pow(10.0, -m_feed / 20.0);

    const double omega = 2.0 * std::numbers::pi * fc / m_sampleRate;
    const double x = std::exp(-omega);

    m_b1_l = x;
    m_a0_l = 1.0 - x;
    m_g_l = g;

    m_b1_r = m_b1_l;
    m_a0_r = m_a0_l;
    m_g_r = m_g_l;

    reset();
}

void Crossfeed::setPreset(CrossfeedPreset preset) {
    switch (preset) {
        case CrossfeedPreset::Default:
            configure(m_sampleRate, 700.0, 4.5);
            break;
        case CrossfeedPreset::Subtle:
            configure(m_sampleRate, 700.0, 6.0);
            break;
        case CrossfeedPreset::Strong:
            configure(m_sampleRate, 650.0, 3.0);
            break;
        case CrossfeedPreset::Custom:
            break;
    }
}

void Crossfeed::reset() noexcept {
    m_z_l = 0.0;
    m_z_r = 0.0;
}

void Crossfeed::process(float* interleavedStereo, size_t frames) noexcept {
    if (!m_enabled || interleavedStereo == nullptr || frames == 0) return;

    for (size_t f = 0; f < frames; ++f) {
        const double l = interleavedStereo[f * 2 + 0];
        const double r = interleavedStereo[f * 2 + 1];

        // Low-pass filter cross-channel components
        m_z_l = m_a0_l * l + m_b1_l * m_z_l;
        m_z_r = m_a0_r * r + m_b1_r * m_z_r;

        // Mix: direct channel plus attenuated low-passed opposite channel
        const double outL = l + m_g_l * m_z_r;
        const double outR = r + m_g_r * m_z_l;

        // Normalize gain to avoid clipping from additive crossfeed
        constexpr double kNorm = 0.85;
        interleavedStereo[f * 2 + 0] = static_cast<float>(outL * kNorm);
        interleavedStereo[f * 2 + 1] = static_cast<float>(outR * kNorm);
    }
}

} // namespace soniq::dsp
