#pragma once

#include "BiquadFilter.hpp"
#include <vector>
#include <string>
#include <array>
#include <memory>

namespace soniq::dsp {

enum class EqualizerMode {
    TenBand = 0,
    ThirtyOneBand,
    Parametric
};

struct ParametricBand {
    FilterType type{FilterType::PeakingEQ};
    double frequency{1000.0};
    double Q{1.0};
    double gainDb{0.0};
    bool enabled{true};
};

class Equalizer {
public:
    static constexpr size_t kNum10Bands = 10;
    static constexpr size_t kNum31Bands = 31;

    static const std::array<double, kNum10Bands>& frequencies10Band();
    static const std::array<double, kNum31Bands>& frequencies31Band();

    Equalizer();

    void setSampleRate(double sampleRate);
    [[nodiscard]] double sampleRate() const noexcept { return m_sampleRate; }

    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }

    void setMode(EqualizerMode mode);
    [[nodiscard]] EqualizerMode mode() const noexcept { return m_mode; }

    void setPreampDb(double gainDb) noexcept;
    [[nodiscard]] double preampDb() const noexcept { return m_preampDb; }

    // 10-Band Controls
    void setBandGain10(size_t band, double gainDb);
    [[nodiscard]] double bandGain10(size_t band) const noexcept;
    const std::array<double, kNum10Bands>& gains10() const noexcept { return m_gains10; }

    // 31-Band Controls
    void setBandGain31(size_t band, double gainDb);
    [[nodiscard]] double bandGain31(size_t band) const noexcept;
    const std::array<double, kNum31Bands>& gains31() const noexcept { return m_gains31; }

    // Parametric Controls
    void setParametricBands(const std::vector<ParametricBand>& bands);
    [[nodiscard]] const std::vector<ParametricBand>& parametricBands() const noexcept { return m_parametricBands; }

    // Presets
    void applyPreset(const std::string& name);
    [[nodiscard]] static std::vector<std::string> presetNames();

    // Reset
    void resetGains() noexcept;
    void resetFilterStates() noexcept;

    // Signal check: does this EQ alter the audio?
    [[nodiscard]] bool altersSignal() const noexcept;

    // Processing
    void process(float* interleavedData, size_t frames, uint16_t channels) noexcept;

    // Response curve for visualizer (gain in dB at arbitrary frequency)
    [[nodiscard]] double gainAtFrequency(double freq) const noexcept;

private:
    void rebuildFilters();

    bool m_enabled{false};
    EqualizerMode m_mode{EqualizerMode::TenBand};
    double m_sampleRate{44100.0};
    double m_preampDb{0.0};
    float m_preampLinear{1.0f};

    std::array<double, kNum10Bands> m_gains10{};
    std::array<double, kNum31Bands> m_gains31{};
    std::vector<ParametricBand> m_parametricBands;

    // Filter banks: indexed by [channel][band]
    // Supports up to 8 channels (stereo is channels 0 and 1)
    static constexpr size_t kMaxChannels = 8;
    std::array<std::vector<BiquadFilter>, kMaxChannels> m_filters;
};

} // namespace soniq::dsp
