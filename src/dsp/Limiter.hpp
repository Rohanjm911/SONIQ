#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>

namespace soniq::dsp {

class Limiter {
public:
    Limiter();

    void configure(double sampleRate, double thresholdDb = -0.3, double releaseTimeMs = 50.0);

    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }

    void setThresholdDb(double db);
    [[nodiscard]] double thresholdDb() const noexcept { return m_thresholdDb; }

    void setCeilingDb(double db);
    [[nodiscard]] double ceilingDb() const noexcept { return m_ceilingDb; }

    void reset() noexcept;

    void process(float* interleavedData, size_t frames, uint16_t channels) noexcept;

    [[nodiscard]] bool wasTriggered() const noexcept { return m_triggered; }
    [[nodiscard]] float currentGainReductionDb() const noexcept;

private:
    bool m_enabled{true};
    double m_sampleRate{44100.0};
    double m_thresholdDb{-0.3};
    double m_ceilingDb{-0.1};
    double m_releaseTimeMs{50.0};

    float m_thresholdLinear{0.966f};
    float m_ceilingLinear{0.988f};
    float m_currentGain{1.0f};
    float m_releaseCoeff{0.999f};

    bool m_triggered{false};
};

} // namespace soniq::dsp
