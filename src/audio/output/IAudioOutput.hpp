#pragma once

#include "core/Types.hpp"
#include "core/AudioFormat.hpp"
#include <string>
#include <vector>
#include <memory>

namespace soniq::audio {

struct AudioDeviceInfo {
    std::string id;
    std::string name;
    bool isDefault{false};
    bool supportsExclusive{false};
    std::vector<uint32_t> supportedSampleRates;
    std::vector<core::SampleFormat> supportedFormats;
};

class IAudioOutput {
public:
    virtual ~IAudioOutput() = default;

    [[nodiscard]] virtual std::string backendName() const noexcept = 0;
    [[nodiscard]] virtual std::vector<AudioDeviceInfo> availableDevices() = 0;
    [[nodiscard]] virtual std::string currentDeviceId() const = 0;

    virtual core::Result<void> open(const core::AudioFormat& format,
                                    const std::string& deviceId = "",
                                    core::OutputMode mode = core::OutputMode::Shared) = 0;

    virtual void close() = 0;

    virtual core::Result<void> start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    // Writes interleaved float32 samples to the audio output device
    // Returns actual number of frames written
    virtual size_t write(const float* data, size_t frames) = 0;

    virtual void setVolume(float volumeLinear) = 0;
    [[nodiscard]] virtual float volume() const noexcept = 0;

    virtual void setMuted(bool muted) = 0;
    [[nodiscard]] virtual bool isMuted() const noexcept = 0;

    [[nodiscard]] virtual core::AudioFormat activeFormat() const noexcept = 0;
    [[nodiscard]] virtual core::OutputMode activeMode() const noexcept = 0;
    [[nodiscard]] virtual core::BitPerfectStatus bitPerfectStatus() const noexcept = 0;
    [[nodiscard]] virtual std::string bitPerfectStatusExplanation() const = 0;

    [[nodiscard]] virtual uint32_t bufferSizeFrames() const noexcept = 0;
    [[nodiscard]] virtual double latencyMs() const noexcept = 0;
    [[nodiscard]] virtual bool isRunning() const noexcept = 0;
};

} // namespace soniq::audio
