#pragma once

#include "audio/AudioDecoder.hpp"
#include "DsdProcessor.hpp"
#include <fstream>
#include <vector>

namespace soniq::audio::dsd {

class DsdAudioDecoder : public AudioDecoder {
public:
    explicit DsdAudioDecoder(DsdPlaybackMode mode = DsdPlaybackMode::DecimatedPcm);
    ~DsdAudioDecoder() override;

    core::Result<void> open(const std::string& filePath) override;
    void close() override;

    size_t readFrames(float* buffer, size_t frames) override;
    core::Result<void> seek(double seconds) override;

    [[nodiscard]] double durationSeconds() const noexcept override { return m_info.durationSeconds; }
    [[nodiscard]] double positionSeconds() const noexcept override;
    [[nodiscard]] core::AudioFormat format() const noexcept override { return m_outputFormat; }
    [[nodiscard]] bool isEof() const noexcept override { return m_eof; }
    [[nodiscard]] bool isOpen() const noexcept override { return m_isOpen; }

    void setPlaybackMode(DsdPlaybackMode mode) noexcept { m_playbackMode = mode; }
    [[nodiscard]] DsdPlaybackMode playbackMode() const noexcept { return m_playbackMode; }
    [[nodiscard]] const DsdFileInfo& dsdInfo() const noexcept { return m_info; }

private:
    void updateOutputFormat();
    size_t readFramesDecimated(float* buffer, size_t frames);
    size_t readFramesDoP(float* buffer, size_t frames);

    DsdPlaybackMode m_playbackMode{DsdPlaybackMode::DecimatedPcm};
    DsdFileInfo m_info;
    core::AudioFormat m_outputFormat;

    std::ifstream m_file;
    bool m_isOpen{false};
    bool m_eof{false};
    uint64_t m_currentSample{0};
    uint8_t m_dopMarkerState{0};

    // Intermediate block reading
    std::vector<uint8_t> m_dsfBlockBuffer;
};

} // namespace soniq::audio::dsd
