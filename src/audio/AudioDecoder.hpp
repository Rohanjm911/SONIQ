#pragma once

#include "core/Types.hpp"
#include "core/AudioFormat.hpp"
#include <string>
#include <memory>

namespace soniq::audio {

class AudioDecoder {
public:
    virtual ~AudioDecoder() = default;

    virtual core::Result<void> open(const std::string& filePath) = 0;
    virtual void close() = 0;

    // Reads up to `frames` into interleaved float32 buffer
    // Returns actual number of frames read (0 indicates EOF or error)
    virtual size_t readFrames(float* buffer, size_t frames) = 0;

    virtual core::Result<void> seek(double seconds) = 0;

    [[nodiscard]] virtual double durationSeconds() const noexcept = 0;
    [[nodiscard]] virtual double positionSeconds() const noexcept = 0;
    [[nodiscard]] virtual core::AudioFormat format() const noexcept = 0;
    [[nodiscard]] virtual bool isEof() const noexcept = 0;
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;
};

} // namespace soniq::audio
