#include "AudioFormat.hpp"
#include <format>
#include <algorithm>

namespace soniq::core {

AudioFormat::AudioFormat(uint32_t sampleRate,
                         uint16_t channels,
                         uint16_t bitDepth,
                         SampleFormat sampleFormat,
                         ChannelLayout layout,
                         std::string codec,
                         std::string container,
                         uint32_t bitrate)
    : m_sampleRate(sampleRate)
    , m_channels(channels)
    , m_bitDepth(bitDepth)
    , m_sampleFormat(sampleFormat)
    , m_channelLayout(layout)
    , m_bitrate(bitrate)
    , m_codec(std::move(codec))
    , m_container(std::move(container)) {}

size_t AudioFormat::bytesPerSample() const noexcept {
    switch (m_sampleFormat) {
        case SampleFormat::Int16:   return 2;
        case SampleFormat::Int24:   return 3;
        case SampleFormat::Int32:
        case SampleFormat::Float32: return 4;
        case SampleFormat::Float64: return 8;
        case SampleFormat::DSD64:
        case SampleFormat::DSD128:
        case SampleFormat::DSD256:  return 1;
        default:
            return (m_bitDepth > 0) ? (m_bitDepth / 8) : 4;
    }
}

size_t AudioFormat::frameSize() const noexcept {
    return bytesPerSample() * (m_channels > 0 ? m_channels : 2);
}

size_t AudioFormat::bytesPerSecond() const noexcept {
    return frameSize() * m_sampleRate;
}

double AudioFormat::framesToSeconds(uint64_t frames) const noexcept {
    if (m_sampleRate == 0) return 0.0;
    return static_cast<double>(frames) / static_cast<double>(m_sampleRate);
}

uint64_t AudioFormat::secondsToFrames(double seconds) const noexcept {
    if (seconds <= 0.0) return 0;
    return static_cast<uint64_t>(seconds * static_cast<double>(m_sampleRate));
}

bool AudioFormat::isLossless() const noexcept {
    std::string upperCodec = m_codec;
    std::transform(upperCodec.begin(), upperCodec.end(), upperCodec.begin(), ::toupper);

    return (upperCodec == "FLAC" ||
            upperCodec == "ALAC" ||
            upperCodec == "WAV" ||
            upperCodec == "AIFF" ||
            upperCodec == "PCM" ||
            upperCodec == "WAVPACK" ||
            upperCodec == "APE" ||
            upperCodec == "MONKEY'S AUDIO" ||
            upperCodec == "TAK" ||
            upperCodec == "TTA" ||
            upperCodec == "DSD" ||
            upperCodec == "DSF" ||
            upperCodec == "DFF");
}

bool AudioFormat::isHiRes() const noexcept {
    // Standard Red Book CD audio is 44.1 kHz / 16-bit
    // Hi-Res is typically > 44.1/48kHz or > 16-bit
    return (m_sampleRate > 48000 || m_bitDepth > 16 || isDsd());
}

bool AudioFormat::isDsd() const noexcept {
    return m_sampleFormat == SampleFormat::DSD64 ||
           m_sampleFormat == SampleFormat::DSD128 ||
           m_sampleFormat == SampleFormat::DSD256 ||
           m_codec == "DSD" || m_codec == "DSF" || m_codec == "DFF";
}

bool AudioFormat::isValid() const noexcept {
    return m_sampleRate >= 8000 && m_sampleRate <= 768000 &&
           m_channels >= 1 && m_channels <= 32 &&
           m_bitDepth >= 8 && m_bitDepth <= 64;
}

bool AudioFormat::isBitPerfectCompatibleWith(const AudioFormat& other) const noexcept {
    // For bit-perfect output, sample rate and channel count must match exactly
    // and sample format must match or be unpadded
    return m_sampleRate == other.m_sampleRate &&
           m_channels == other.m_channels &&
           m_bitDepth == other.m_bitDepth &&
           m_sampleFormat == other.m_sampleFormat;
}

std::string AudioFormat::toString() const {
    double khz = static_cast<double>(m_sampleRate) / 1000.0;
    return std::format("{} / {:.1f} kHz / {}-bit", m_codec, khz, m_bitDepth);
}

std::string AudioFormat::toDetailedString() const {
    double khz = static_cast<double>(m_sampleRate) / 1000.0;
    std::string chLayoutStr = (m_channels == 1) ? "Mono" : (m_channels == 2) ? "Stereo" : std::format("{} ch", m_channels);
    std::string brStr = (m_bitrate > 0) ? std::format(" / {} kbps", m_bitrate) : "";
    return std::format("{} [{}] - {:.1f} kHz, {}-bit, {}{}",
                       m_codec, m_container, khz, m_bitDepth, chLayoutStr, brStr);
}

bool AudioFormat::operator==(const AudioFormat& other) const noexcept {
    return m_sampleRate == other.m_sampleRate &&
           m_channels == other.m_channels &&
           m_bitDepth == other.m_bitDepth &&
           m_sampleFormat == other.m_sampleFormat &&
           m_channelLayout == other.m_channelLayout &&
           m_codec == other.m_codec &&
           m_container == other.m_container;
}

bool AudioFormat::operator!=(const AudioFormat& other) const noexcept {
    return !(*this == other);
}

} // namespace soniq::core
