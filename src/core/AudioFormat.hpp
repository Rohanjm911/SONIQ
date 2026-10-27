#pragma once

#include "Types.hpp"
#include <string>
#include <string_view>
#include <cstdint>

namespace soniq::core {

class AudioFormat {
public:
    AudioFormat() = default;

    AudioFormat(uint32_t sampleRate,
                uint16_t channels,
                uint16_t bitDepth,
                SampleFormat sampleFormat = SampleFormat::Float32,
                ChannelLayout layout = ChannelLayout::Stereo,
                std::string codec = "PCM",
                std::string container = "WAV",
                uint32_t bitrate = 0);

    [[nodiscard]] uint32_t sampleRate() const noexcept { return m_sampleRate; }
    [[nodiscard]] uint16_t channels() const noexcept { return m_channels; }
    [[nodiscard]] uint16_t bitDepth() const noexcept { return m_bitDepth; }
    [[nodiscard]] SampleFormat sampleFormat() const noexcept { return m_sampleFormat; }
    [[nodiscard]] ChannelLayout channelLayout() const noexcept { return m_channelLayout; }
    [[nodiscard]] uint32_t bitrate() const noexcept { return m_bitrate; }
    [[nodiscard]] const std::string& codec() const noexcept { return m_codec; }
    [[nodiscard]] const std::string& container() const noexcept { return m_container; }

    void setSampleRate(uint32_t rate) noexcept { m_sampleRate = rate; }
    void setChannels(uint16_t ch) noexcept { m_channels = ch; }
    void setBitDepth(uint16_t depth) noexcept { m_bitDepth = depth; }
    void setSampleFormat(SampleFormat fmt) noexcept { m_sampleFormat = fmt; }
    void setChannelLayout(ChannelLayout layout) noexcept { m_channelLayout = layout; }
    void setBitrate(uint32_t br) noexcept { m_bitrate = br; }
    void setCodec(std::string codec) { m_codec = std::move(codec); }
    void setContainer(std::string container) { m_container = std::move(container); }

    [[nodiscard]] size_t bytesPerSample() const noexcept;
    [[nodiscard]] size_t frameSize() const noexcept;
    [[nodiscard]] size_t bytesPerSecond() const noexcept;

    [[nodiscard]] double framesToSeconds(uint64_t frames) const noexcept;
    [[nodiscard]] uint64_t secondsToFrames(double seconds) const noexcept;

    [[nodiscard]] bool isLossless() const noexcept;
    [[nodiscard]] bool isHiRes() const noexcept;
    [[nodiscard]] bool isDsd() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] bool isBitPerfectCompatibleWith(const AudioFormat& other) const noexcept;

    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string toDetailedString() const;

    bool operator==(const AudioFormat& other) const noexcept;
    bool operator!=(const AudioFormat& other) const noexcept;

private:
    uint32_t m_sampleRate{44100};
    uint16_t m_channels{2};
    uint16_t m_bitDepth{16};
    SampleFormat m_sampleFormat{SampleFormat::Float32};
    ChannelLayout m_channelLayout{ChannelLayout::Stereo};
    uint32_t m_bitrate{0}; // in kbps
    std::string m_codec{"PCM"};
    std::string m_container{"WAV"};
};

} // namespace soniq::core
