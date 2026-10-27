#pragma once

#include "IAudioOutput.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <atomic>
#include <mutex>
#include <thread>

namespace soniq::audio {

class WASAPIAudioOutput : public IAudioOutput {
public:
    WASAPIAudioOutput();
    ~WASAPIAudioOutput() override;

    [[nodiscard]] std::string backendName() const noexcept override { return "Windows Core Audio (WASAPI)"; }
    [[nodiscard]] std::vector<AudioDeviceInfo> availableDevices() override;
    [[nodiscard]] std::string currentDeviceId() const override { return m_currentDeviceId; }

    core::Result<void> open(const core::AudioFormat& format,
                            const std::string& deviceId = "",
                            core::OutputMode mode = core::OutputMode::Shared) override;

    void close() override;

    core::Result<void> start() override;
    void pause() override;
    void stop() override;

    size_t write(const float* data, size_t frames) override;

    void setVolume(float volumeLinear) override;
    [[nodiscard]] float volume() const noexcept override { return m_volume; }

    void setMuted(bool muted) override;
    [[nodiscard]] bool isMuted() const noexcept override { return m_muted; }

    [[nodiscard]] core::AudioFormat activeFormat() const noexcept override { return m_format; }
    [[nodiscard]] core::OutputMode activeMode() const noexcept override { return m_mode; }
    [[nodiscard]] core::BitPerfectStatus bitPerfectStatus() const noexcept override;
    [[nodiscard]] std::string bitPerfectStatusExplanation() const override;

    [[nodiscard]] uint32_t bufferSizeFrames() const noexcept override { return m_bufferFrameCount; }
    [[nodiscard]] double latencyMs() const noexcept override { return m_latencyMs; }
    [[nodiscard]] bool isRunning() const noexcept override { return m_running; }

private:
    void releaseCOM() noexcept;

    core::AudioFormat m_format;
    core::OutputMode m_mode{core::OutputMode::Shared};
    std::string m_currentDeviceId;

    std::atomic<bool> m_running{false};
    std::atomic<float> m_volume{1.0f};
    std::atomic<bool> m_muted{false};
    std::atomic<bool> m_exclusiveAcquired{false};

    uint32_t m_bufferFrameCount{0};
    double m_latencyMs{0.0};

    // Windows Audio Client COM pointers
    IMMDeviceEnumerator* m_deviceEnumerator{nullptr};
    IMMDevice* m_device{nullptr};
    IAudioClient* m_audioClient{nullptr};
    IAudioRenderClient* m_renderClient{nullptr};
    HANDLE m_audioEvent{nullptr};

    mutable std::recursive_mutex m_mutex;
    bool m_coInitialized{false};
};

} // namespace soniq::audio

#endif // _WIN32
