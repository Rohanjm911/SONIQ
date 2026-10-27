#include "DsdAudioDecoder.hpp"
#include "core/Logger.hpp"
#include <algorithm>
#include <cmath>

namespace soniq::audio::dsd {

static uint8_t reverseBits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

DsdAudioDecoder::DsdAudioDecoder(DsdPlaybackMode mode)
    : m_playbackMode(mode) {}

DsdAudioDecoder::~DsdAudioDecoder() {
    close();
}

core::Result<void> DsdAudioDecoder::open(const std::string& filePath) {
    close();

    if (!DsdParser::parseFile(filePath, m_info)) {
        return core::Error::make(-1, "Failed to parse DSD (DSF/DFF) file header", "DsdDecoder");
    }

    m_file.open(filePath, std::ios::binary);
    if (!m_file.is_open()) {
        return core::Error::make(-2, "Failed to open DSD audio file", "DsdDecoder");
    }

    m_file.seekg(m_info.dataOffset, std::ios::beg);
    m_dsfBlockBuffer.resize(m_info.blockSize * (m_info.channels > 0 ? m_info.channels : 2));

    updateOutputFormat();

    m_isOpen = true;
    m_eof = false;
    m_currentSample = 0;
    m_dopMarkerState = 0;

    SONIQ_LOG_INFO("Opened DSD stream: {} channels, {} Hz, duration {:.2f}s, mode: {}",
                   m_info.channels, m_info.sampleRate, m_info.durationSeconds,
                   m_playbackMode == DsdPlaybackMode::DoP ? "DoP (DSD over PCM)" : "Decimated PCM");

    return core::Result<void>::ok();
}

void DsdAudioDecoder::close() {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_isOpen = false;
    m_eof = false;
    m_currentSample = 0;
    m_dopMarkerState = 0;
}

void DsdAudioDecoder::updateOutputFormat() {
    uint16_t ch = m_info.channels > 0 ? m_info.channels : 2;
    auto layout = (ch == 1) ? core::ChannelLayout::Mono : core::ChannelLayout::Stereo;
    std::string container = (m_info.type == DsdFormatType::DSF) ? "DSF" : "DFF";

    if (m_playbackMode == DsdPlaybackMode::DoP) {
        // DoP framerate is DSD rate / 16 (176.4 kHz for DSD64)
        uint32_t dopSampleRate = m_info.sampleRate / 16;
        m_outputFormat = core::AudioFormat(dopSampleRate, ch, 24,
                                           core::SampleFormat::Float32, layout,
                                           "DSD (DoP)", container,
                                           (m_info.sampleRate * ch) / 1000);
    } else {
        // 32:1 Decimation (88.2 kHz for DSD64)
        uint32_t decSampleRate = m_info.sampleRate / 32;
        m_outputFormat = core::AudioFormat(decSampleRate, ch, 32,
                                           core::SampleFormat::Float32, layout,
                                           "DSD (Decimated)", container,
                                           (m_info.sampleRate * ch) / 1000);
    }
}

double DsdAudioDecoder::positionSeconds() const noexcept {
    if (m_info.sampleRate == 0) return 0.0;
    return static_cast<double>(m_currentSample) / static_cast<double>(m_info.sampleRate);
}

core::Result<void> DsdAudioDecoder::seek(double seconds) {
    if (!m_isOpen) {
        return core::Error::make(-1, "Decoder not open", "DsdDecoder");
    }

    uint64_t targetSample = static_cast<uint64_t>(seconds * m_info.sampleRate);
    if (targetSample > m_info.sampleCount) {
        targetSample = m_info.sampleCount;
    }

    uint16_t ch = m_info.channels > 0 ? m_info.channels : 2;
    if (m_info.type == DsdFormatType::DSF) {
        uint64_t samplesPerBlock = static_cast<uint64_t>(m_info.blockSize) * 8;
        uint64_t targetBlock = targetSample / samplesPerBlock;
        uint64_t blockBytes = static_cast<uint64_t>(m_info.blockSize) * ch;
        uint64_t fileOffset = m_info.dataOffset + targetBlock * blockBytes;

        m_file.clear();
        m_file.seekg(fileOffset, std::ios::beg);
        m_currentSample = targetBlock * samplesPerBlock;
    } else {
        // DFF interleaved bytes
        uint64_t byteIdx = (targetSample / 8) * ch;
        m_file.clear();
        m_file.seekg(m_info.dataOffset + byteIdx, std::ios::beg);
        m_currentSample = (byteIdx / ch) * 8;
    }

    m_eof = false;
    m_dopMarkerState = 0;
    return core::Result<void>::ok();
}

size_t DsdAudioDecoder::readFrames(float* buffer, size_t frames) {
    if (!m_isOpen || m_eof || !buffer || frames == 0) return 0;

    if (m_playbackMode == DsdPlaybackMode::DoP) {
        return readFramesDoP(buffer, frames);
    } else {
        return readFramesDecimated(buffer, frames);
    }
}

size_t DsdAudioDecoder::readFramesDoP(float* buffer, size_t frames) {
    // 1 DoP frame = 16 DSD bits per channel = 2 bytes per channel
    uint16_t ch = m_info.channels > 0 ? m_info.channels : 2;
    std::vector<uint16_t> ch0Words(frames, 0x6969); // DSD silence is 0x69 or 0x96
    std::vector<uint16_t> ch1Words(frames, 0x6969);

    size_t framesRead = 0;

    for (size_t f = 0; f < frames; ++f) {
        if (m_currentSample >= m_info.sampleCount || !m_file.good()) {
            m_eof = true;
            break;
        }

        uint8_t b0[2] = {0x69, 0x69};
        uint8_t b1[2] = {0x69, 0x69};

        if (m_info.type == DsdFormatType::DSF) {
            // For DSF, read 2 bytes per channel
            m_file.read(reinterpret_cast<char*>(b0), 2);
            if (ch > 1) {
                m_file.read(reinterpret_cast<char*>(b1), 2);
            } else {
                b1[0] = b0[0]; b1[1] = b0[1];
            }
        } else {
            // DFF byte interleaved
            m_file.read(reinterpret_cast<char*>(b0), 1);
            if (ch > 1) m_file.read(reinterpret_cast<char*>(b1), 1);
            m_file.read(reinterpret_cast<char*>(b0 + 1), 1);
            if (ch > 1) m_file.read(reinterpret_cast<char*>(b1 + 1), 1);
        }

        if (m_info.lsbFirst) {
            // DoP expects MSB first bit-transmission
            b0[0] = reverseBits(b0[0]);
            b0[1] = reverseBits(b0[1]);
            b1[0] = reverseBits(b1[0]);
            b1[1] = reverseBits(b1[1]);
        }

        ch0Words[f] = (static_cast<uint16_t>(b0[0]) << 8) | b0[1];
        ch1Words[f] = (static_cast<uint16_t>(b1[0]) << 8) | b1[1];

        m_currentSample += 16;
        framesRead++;
    }

    if (framesRead > 0) {
        DsdProcessor::packDoPFloat(ch0Words.data(), ch1Words.data(), buffer, framesRead, m_dopMarkerState);
    }

    return framesRead;
}

size_t DsdAudioDecoder::readFramesDecimated(float* buffer, size_t frames) {
    // 32:1 decimation factor: 32 DSD bits = 4 bytes per channel for 1 PCM sample
    constexpr size_t kDecimation = 32;
    uint16_t ch = m_info.channels > 0 ? m_info.channels : 2;

    size_t dsdBytesNeeded = frames * 4;
    std::vector<uint8_t> ch0Bytes(dsdBytesNeeded, 0x69);
    std::vector<uint8_t> ch1Bytes(dsdBytesNeeded, 0x69);

    size_t bytesRead = 0;
    while (bytesRead < dsdBytesNeeded && m_currentSample < m_info.sampleCount && m_file.good()) {
        size_t toRead = std::min(size_t(512), dsdBytesNeeded - bytesRead);
        m_file.read(reinterpret_cast<char*>(ch0Bytes.data() + bytesRead), toRead);
        size_t actual = m_file.gcount();
        if (ch > 1) {
            m_file.read(reinterpret_cast<char*>(ch1Bytes.data() + bytesRead), actual);
        } else {
            std::memcpy(ch1Bytes.data() + bytesRead, ch0Bytes.data() + bytesRead, actual);
        }
        bytesRead += actual;
        m_currentSample += actual * 8;
        if (actual < toRead) break;
    }

    if (bytesRead == 0 || m_currentSample >= m_info.sampleCount) {
        m_eof = true;
        if (bytesRead == 0) return 0;
    }

    std::vector<float> pcmL(frames, 0.0f);
    std::vector<float> pcmR(frames, 0.0f);
    size_t outSamplesL = 0;
    size_t outSamplesR = 0;

    DsdProcessor::decimateDsdToPcm(ch0Bytes.data(), bytesRead, m_info.lsbFirst, pcmL.data(), outSamplesL, kDecimation);
    DsdProcessor::decimateDsdToPcm(ch1Bytes.data(), bytesRead, m_info.lsbFirst, pcmR.data(), outSamplesR, kDecimation);

    size_t totalFrames = std::min(outSamplesL, frames);
    for (size_t f = 0; f < totalFrames; ++f) {
        buffer[f * ch + 0] = pcmL[f];
        if (ch > 1) {
            buffer[f * ch + 1] = pcmR[f];
        }
    }

    return totalFrames;
}

} // namespace soniq::audio::dsd
