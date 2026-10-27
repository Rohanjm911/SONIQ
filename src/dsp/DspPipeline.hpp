#pragma once

#include "Equalizer.hpp"
#include "Limiter.hpp"
#include "Crossfeed.hpp"
#include "core/Types.hpp"
#include <vector>
#include <string>
#include <memory>
#include <atomic>

namespace soniq::dsp {

class DspPipeline {
public:
    DspPipeline();

    void setSampleRate(double sampleRate);
    [[nodiscard]] double sampleRate() const noexcept { return m_sampleRate; }

    void setMasterBypass(bool bypass) noexcept { m_masterBypass = bypass; }
    [[nodiscard]] bool masterBypass() const noexcept { return m_masterBypass; }

    // Equalizer
    [[nodiscard]] Equalizer& equalizer() noexcept { return m_equalizer; }
    [[nodiscard]] const Equalizer& equalizer() const noexcept { return m_equalizer; }

    // Limiter
    [[nodiscard]] Limiter& limiter() noexcept { return m_limiter; }
    [[nodiscard]] const Limiter& limiter() const noexcept { return m_limiter; }

    // Crossfeed
    [[nodiscard]] Crossfeed& crossfeed() noexcept { return m_crossfeed; }
    [[nodiscard]] const Crossfeed& crossfeed() const noexcept { return m_crossfeed; }

    // Stereo Balance (-1.0 left to +1.0 right)
    void setStereoBalance(float balance) noexcept;
    [[nodiscard]] float stereoBalance() const noexcept { return m_stereoBalance; }

    // Stereo Width (0.0 mono, 1.0 normal, 2.0 enhanced)
    void setStereoWidth(float width) noexcept;
    [[nodiscard]] float stereoWidth() const noexcept { return m_stereoWidth; }

    // ReplayGain
    void setReplayGainEnabled(bool enabled) noexcept { m_replayGainEnabled = enabled; }
    [[nodiscard]] bool replayGainEnabled() const noexcept { return m_replayGainEnabled; }
    void setTrackReplayGainDb(float gainDb) noexcept;
    void setPreampGainDb(float gainDb) noexcept;

    // Signal alterations check for Bit-Perfect verification
    [[nodiscard]] bool altersSignal() const noexcept;
    [[nodiscard]] std::vector<std::string> bitPerfectDisablingReasons() const;

    // Reset all internal states (e.g., when track changes or seek occurs)
    void reset() noexcept;

    // Process audio block in-place
    void process(float* interleavedData, size_t frames, uint16_t channels) noexcept;

private:
    double m_sampleRate{44100.0};
    bool m_masterBypass{false};

    Equalizer m_equalizer;
    Limiter m_limiter;
    Crossfeed m_crossfeed;

    float m_stereoBalance{0.0f}; // 0 = center
    float m_leftGain{1.0f};
    float m_rightGain{1.0f};

    float m_stereoWidth{1.0f}; // 1.0 = normal stereo

    bool m_replayGainEnabled{false};
    float m_replayGainDb{0.0f};
    float m_replayGainLinear{1.0f};

    float m_preampGainDb{0.0f};
    float m_preampGainLinear{1.0f};
};

} // namespace soniq::dsp
