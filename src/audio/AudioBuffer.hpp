#pragma once

#include "core/Types.hpp"
#include "core/AudioFormat.hpp"
#include <vector>
#include <cstddef>
#include <span>

namespace soniq::audio {

class AudioBuffer {
public:
    AudioBuffer() = default;
    explicit AudioBuffer(size_t frames, uint16_t channels = 2);
    AudioBuffer(const float* interleavedData, size_t frames, uint16_t channels);

    void resize(size_t frames, uint16_t channels = 2);
    void clear() noexcept;

    [[nodiscard]] size_t frames() const noexcept { return m_frames; }
    [[nodiscard]] uint16_t channels() const noexcept { return m_channels; }
    [[nodiscard]] size_t totalSamples() const noexcept { return m_frames * m_channels; }
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

    [[nodiscard]] float* data() noexcept { return m_data.data(); }
    [[nodiscard]] const float* data() const noexcept { return m_data.data(); }

    [[nodiscard]] float sample(size_t frame, uint16_t channel) const noexcept {
        return m_data[frame * m_channels + channel];
    }

    float& sample(size_t frame, uint16_t channel) noexcept {
        return m_data[frame * m_channels + channel];
    }

    void applyGain(float gainLinear) noexcept;
    void applyGainRamp(float startGain, float endGain) noexcept;

    [[nodiscard]] float peakLevel() const noexcept;
    [[nodiscard]] float peakLevelChannel(uint16_t channel) const noexcept;
    [[nodiscard]] float rmsLevel() const noexcept;
    [[nodiscard]] bool hasClipping(float threshold = 1.0f) const noexcept;

    // Conversion helpers from various raw sample formats to internal Float32
    void loadFromInt16(const int16_t* src, size_t frames, uint16_t channels);
    void loadFromInt24(const uint8_t* src, size_t frames, uint16_t channels);
    void loadFromInt32(const int32_t* src, size_t frames, uint16_t channels);
    void loadFromFloat32(const float* src, size_t frames, uint16_t channels);

    // Conversion helpers from internal Float32 to destination formats
    void copyToInt16(int16_t* dst) const noexcept;
    void copyToInt24(uint8_t* dst) const noexcept;
    void copyToInt32(int32_t* dst) const noexcept;
    void copyToFloat32(float* dst) const noexcept;

    std::span<float> asSpan() noexcept { return std::span<float>(m_data); }
    std::span<const float> asSpan() const noexcept { return std::span<const float>(m_data); }

private:
    size_t m_frames{0};
    uint16_t m_channels{2};
    std::vector<float> m_data;
};

} // namespace soniq::audio
