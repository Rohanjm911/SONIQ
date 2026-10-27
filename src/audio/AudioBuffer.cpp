#include "AudioBuffer.hpp"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace soniq::audio {

AudioBuffer::AudioBuffer(size_t frames, uint16_t channels)
    : m_frames(frames)
    , m_channels(channels)
    , m_data(frames * channels, 0.0f) {}

AudioBuffer::AudioBuffer(const float* interleavedData, size_t frames, uint16_t channels)
    : m_frames(frames)
    , m_channels(channels)
    , m_data(frames * channels) {
    if (interleavedData) {
        std::memcpy(m_data.data(), interleavedData, frames * channels * sizeof(float));
    } else {
        std::fill(m_data.begin(), m_data.end(), 0.0f);
    }
}

void AudioBuffer::resize(size_t frames, uint16_t channels) {
    m_frames = frames;
    m_channels = channels;
    m_data.resize(frames * channels, 0.0f);
}

void AudioBuffer::clear() noexcept {
    std::fill(m_data.begin(), m_data.end(), 0.0f);
}

void AudioBuffer::applyGain(float gainLinear) noexcept {
    if (std::abs(gainLinear - 1.0f) < 1e-6f) return;
    for (float& s : m_data) {
        s *= gainLinear;
    }
}

void AudioBuffer::applyGainRamp(float startGain, float endGain) noexcept {
    if (m_frames == 0) return;
    const float step = (endGain - startGain) / static_cast<float>(m_frames);
    float currentGain = startGain;

    for (size_t f = 0; f < m_frames; ++f) {
        for (uint16_t c = 0; c < m_channels; ++c) {
            m_data[f * m_channels + c] *= currentGain;
        }
        currentGain += step;
    }
}

float AudioBuffer::peakLevel() const noexcept {
    float peak = 0.0f;
    for (float s : m_data) {
        float absVal = std::abs(s);
        if (absVal > peak) peak = absVal;
    }
    return peak;
}

float AudioBuffer::peakLevelChannel(uint16_t channel) const noexcept {
    if (channel >= m_channels) return 0.0f;
    float peak = 0.0f;
    for (size_t f = 0; f < m_frames; ++f) {
        float absVal = std::abs(m_data[f * m_channels + channel]);
        if (absVal > peak) peak = absVal;
    }
    return peak;
}

float AudioBuffer::rmsLevel() const noexcept {
    if (m_data.empty()) return 0.0f;
    double sumSq = 0.0;
    for (float s : m_data) {
        sumSq += static_cast<double>(s) * static_cast<double>(s);
    }
    return static_cast<float>(std::sqrt(sumSq / static_cast<double>(m_data.size())));
}

bool AudioBuffer::hasClipping(float threshold) const noexcept {
    for (float s : m_data) {
        if (std::abs(s) >= threshold) return true;
    }
    return false;
}

void AudioBuffer::loadFromInt16(const int16_t* src, size_t frames, uint16_t channels) {
    resize(frames, channels);
    constexpr float kInvScale = 1.0f / 32768.0f;
    const size_t total = frames * channels;
    for (size_t i = 0; i < total; ++i) {
        m_data[i] = static_cast<float>(src[i]) * kInvScale;
    }
}

void AudioBuffer::loadFromInt24(const uint8_t* src, size_t frames, uint16_t channels) {
    resize(frames, channels);
    constexpr float kInvScale = 1.0f / 8388608.0f; // 2^23
    const size_t total = frames * channels;
    for (size_t i = 0; i < total; ++i) {
        const size_t byteIdx = i * 3;
        // Little endian 24-bit sign-extended
        int32_t val = (static_cast<int32_t>(src[byteIdx])) |
                      (static_cast<int32_t>(src[byteIdx + 1]) << 8) |
                      (static_cast<int32_t>(static_cast<int8_t>(src[byteIdx + 2])) << 16);
        m_data[i] = static_cast<float>(val) * kInvScale;
    }
}

void AudioBuffer::loadFromInt32(const int32_t* src, size_t frames, uint16_t channels) {
    resize(frames, channels);
    constexpr double kInvScale = 1.0 / 2147483648.0;
    const size_t total = frames * channels;
    for (size_t i = 0; i < total; ++i) {
        m_data[i] = static_cast<float>(static_cast<double>(src[i]) * kInvScale);
    }
}

void AudioBuffer::loadFromFloat32(const float* src, size_t frames, uint16_t channels) {
    resize(frames, channels);
    std::memcpy(m_data.data(), src, frames * channels * sizeof(float));
}

void AudioBuffer::copyToInt16(int16_t* dst) const noexcept {
    const size_t total = m_data.size();
    for (size_t i = 0; i < total; ++i) {
        float val = std::clamp(m_data[i], -1.0f, 1.0f);
        dst[i] = static_cast<int16_t>(val >= 0.0f ? val * 32767.0f : val * 32768.0f);
    }
}

void AudioBuffer::copyToInt24(uint8_t* dst) const noexcept {
    const size_t total = m_data.size();
    for (size_t i = 0; i < total; ++i) {
        float val = std::clamp(m_data[i], -1.0f, 1.0f);
        int32_t sample = static_cast<int32_t>(val >= 0.0f ? val * 8388607.0f : val * 8388608.0f);
        dst[i * 3 + 0] = static_cast<uint8_t>(sample & 0xFF);
        dst[i * 3 + 1] = static_cast<uint8_t>((sample >> 8) & 0xFF);
        dst[i * 3 + 2] = static_cast<uint8_t>((sample >> 16) & 0xFF);
    }
}

void AudioBuffer::copyToInt32(int32_t* dst) const noexcept {
    const size_t total = m_data.size();
    for (size_t i = 0; i < total; ++i) {
        double val = std::clamp(static_cast<double>(m_data[i]), -1.0, 1.0);
        dst[i] = static_cast<int32_t>(val >= 0.0 ? val * 2147483647.0 : val * 2147483648.0);
    }
}

void AudioBuffer::copyToFloat32(float* dst) const noexcept {
    std::memcpy(dst, m_data.data(), m_data.size() * sizeof(float));
}

} // namespace soniq::audio
