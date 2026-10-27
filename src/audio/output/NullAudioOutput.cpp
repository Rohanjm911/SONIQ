#include "NullAudioOutput.hpp"
#include <thread>
#include <cmath>

namespace soniq::audio {

NullAudioOutput::NullAudioOutput() = default;
NullAudioOutput::~NullAudioOutput() {
    close();
}

std::vector<AudioDeviceInfo> NullAudioOutput::availableDevices() {
    AudioDeviceInfo dev;
    dev.id = "null_default";
    dev.name = "SONIQ Virtual Bit-Exact Sink (Testing)";
    dev.isDefault = true;
    dev.supportsExclusive = true;
    dev.supportedSampleRates = {44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000};
    dev.supportedFormats = {core::SampleFormat::Int16, core::SampleFormat::Int24, core::SampleFormat::Int32, core::SampleFormat::Float32};
    return {dev};
}

core::Result<void> NullAudioOutput::open(const core::AudioFormat& format,
                                        const std::string& /*deviceId*/,
                                        core::OutputMode mode) {
    if (!format.isValid()) {
        return core::Error::make(-1, "Invalid audio format provided to NullAudioOutput", "NullOutput");
    }

    m_format = format;
    m_mode = mode;
    m_running = false;
    m_totalFramesWritten = 0;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_capturedSamples.clear();
    return core::Result<void>::ok();
}

void NullAudioOutput::close() {
    stop();
}

core::Result<void> NullAudioOutput::start() {
    m_running = true;
    return core::Result<void>::ok();
}

void NullAudioOutput::pause() {
    m_running = false;
}

void NullAudioOutput::stop() {
    m_running = false;
}

size_t NullAudioOutput::write(const float* data, size_t frames) {
    if (!m_running || data == nullptr || frames == 0) return 0;

    const size_t channels = m_format.channels() > 0 ? m_format.channels() : 2;
    const size_t numSamples = frames * channels;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_capturedSamples.insert(m_capturedSamples.end(), data, data + numSamples);
    }

    m_totalFramesWritten += frames;

    if (m_simulateRealTime && m_format.sampleRate() > 0) {
        // Sleep for the duration of the audio block to simulate hardware clock
        double blockSeconds = static_cast<double>(frames) / static_cast<double>(m_format.sampleRate());
        std::this_thread::sleep_for(std::chrono::duration<double>(blockSeconds));
    }

    return frames;
}

core::BitPerfectStatus NullAudioOutput::bitPerfectStatus() const noexcept {
    if (m_mode == core::OutputMode::BitPerfect || m_mode == core::OutputMode::Exclusive) {
        // In bit-perfect mode, if volume is 100% and not muted, bit-perfect output is verified
        if (std::abs(m_volume.load() - 1.0f) < 1e-4f && !m_muted.load()) {
            return core::BitPerfectStatus::Verified;
        }
        return core::BitPerfectStatus::Unavailable;
    }
    return core::BitPerfectStatus::Available;
}

std::string NullAudioOutput::bitPerfectStatusExplanation() const {
    if (m_mode == core::OutputMode::BitPerfect || m_mode == core::OutputMode::Exclusive) {
        if (std::abs(m_volume.load() - 1.0f) >= 1e-4f) {
            return "Volume is attenuated below 100% (software volume alters sample amplitudes)";
        }
        if (m_muted.load()) {
            return "Audio output is muted";
        }
        return "Bit-perfect mode verified: hardware matches stream rate, unresampled bitstream delivery";
    }
    return "Shared mode active: OS audio mixer may resample or dither samples. Enable Exclusive Mode for bit-perfect output.";
}

void NullAudioOutput::clearCaptured() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_capturedSamples.clear();
    m_totalFramesWritten = 0;
}

} // namespace soniq::audio
