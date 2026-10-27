#pragma once

#include "IAudioOutput.hpp"
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>

namespace soniq::audio {

class NullAudioOutput : public IAudioOutput {
public:
    NullAudioOutput();
    ~NullAudioOutput() override;

    [[nodiscard]] std::string backendName() const noexcept override { return "Null / In-Memory Audio"; }
    [[nodiscard]] std::vector<AudioDeviceInfo> availableDevices() override;
    [[nodiscard]] std::string currentDeviceId() const override { return "null_default"; }

    core::Result<void> open(const core::AudioFormat& format,
                            const std::string& deviceId = "",
                            core::OutputMode mode = core::OutputMode::Shared) override;

    void close() override;

    core::Result<void> start() override;
    void pause() override;
    void stop() override;

    size_t write(const float* data, size_t frames) override;

    void setVolume(float volumeLinear) override { m_volume = volumeLinear; }
    [[nodiscard]] float volume() const noexcept override { return m_volume; }

    void setMuted(bool muted) override { m_muted = muted; }
    [[nodiscard]] bool isMuted() const noexcept override { return m_muted; }

    [[nodiscard]] core::AudioFormat activeFormat() const noexcept override { return m_format; }
    [[nodiscard]] core::OutputMode activeMode() const noexcept override { return m_mode; }
    [[nodiscard]] core::BitPerfectStatus bitPerfectStatus() const noexcept override;
    [[nodiscard]] std::string bitPerfectStatusExplanation() const override;

    [[nodiscard]] uint32_t bufferSizeFrames() const noexcept override { return 1024; }
    [[nodiscard]] double latencyMs() const noexcept override { return 10.0; }
    [[nodiscard]] bool isRunning() const noexcept override { return m_running; }

    // Test inspection utilities
    [[nodiscard]] uint64_t totalFramesWritten() const noexcept { return m_totalFramesWritten; }
    [[nodiscard]] const std::vector<float>& capturedSamples() const noexcept { return m_capturedSamples; }
    void clearCaptured() noexcept;
    void setSimulateRealTimePacing(bool enable) noexcept { m_simulateRealTime = enable; }

private:
    core::AudioFormat m_format;
    core::OutputMode m_mode{core::OutputMode::Shared};
    std::atomic<bool> m_running{false};
    std::atomic<float> m_volume{1.0f};
    std::atomic<bool> m_muted{false};
    std::atomic<uint64_t> m_totalFramesWritten{0};
    bool m_simulateRealTime{false};

    mutable std::mutex m_mutex;
    std::vector<float> m_capturedSamples;
};

} // namespace soniq::audio
