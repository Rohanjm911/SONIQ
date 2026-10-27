#pragma once

#include "AudioDecoder.hpp"
#include "metadata/AudioProbe.hpp"
#include <fstream>
#include <vector>
#include <atomic>
#include <cstdio>

namespace soniq::audio {

class FFmpegAudioDecoder : public AudioDecoder {
public:
    FFmpegAudioDecoder();
    ~FFmpegAudioDecoder() override;

    core::Result<void> open(const std::string& filePath) override;
    void close() override;

    size_t readFrames(float* buffer, size_t frames) override;
    core::Result<void> seek(double seconds) override;

    [[nodiscard]] double durationSeconds() const noexcept override { return m_durationSeconds; }
    [[nodiscard]] double positionSeconds() const noexcept override;
    [[nodiscard]] core::AudioFormat format() const noexcept override { return m_format; }
    [[nodiscard]] bool isEof() const noexcept override { return m_eof; }
    [[nodiscard]] bool isOpen() const noexcept override { return m_isOpen; }

private:
    bool openWavDirect(const std::string& filePath);
    bool openFfmpegPipe(const std::string& filePath, double startSeconds = 0.0);
    void closePipe();

    std::string m_filePath;
    core::AudioFormat m_format;
    double m_durationSeconds{0.0};
    uint64_t m_currentFrame{0};
    bool m_isOpen{false};
    bool m_eof{false};

    // Fast-path direct WAV decoder
    bool m_isWavDirect{false};
    std::ifstream m_wavFile;
    std::streampos m_wavDataStart{0};
    size_t m_wavDataBytes{0};
    uint16_t m_wavBitsPerSample{16};
    uint16_t m_wavAudioFormat{1}; // 1 = PCM, 3 = IEEE Float

    // FFmpeg streaming pipe
    FILE* m_pipeHandle{nullptr};
    void* m_processHandle{nullptr};
};

} // namespace soniq::audio
