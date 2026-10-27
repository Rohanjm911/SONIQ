#include "DsdProcessor.hpp"
#include "core/Logger.hpp"
#include <cmath>
#include <cstring>
#include <numbers>
#include <algorithm>

namespace soniq::audio::dsd {

// Precalculated 64-tap symmetric FIR low-pass filter coefficients for 32:1 decimation (40 kHz cutoff)
static std::vector<float> generateFirKernel(size_t taps, double cutoffRatio) {
    std::vector<float> kernel(taps);
    double sum = 0.0;
    int m = static_cast<int>(taps) - 1;

    for (int i = 0; i < static_cast<int>(taps); ++i) {
        double n = i - m / 2.0;
        double sinc = (std::abs(n) < 1e-7) ? 1.0 : (std::sin(2.0 * std::numbers::pi * cutoffRatio * n) / (std::numbers::pi * n));
        // Blackman window
        double w = 0.42 - 0.5 * std::cos(2.0 * std::numbers::pi * i / m) + 0.08 * std::cos(4.0 * std::numbers::pi * i / m);
        kernel[i] = static_cast<float>(sinc * w);
        sum += kernel[i];
    }

    if (sum > 0.0) {
        for (auto& k : kernel) {
            k = static_cast<float>(k / sum);
        }
    }
    return kernel;
}

static const std::vector<float> s_firKernel = generateFirKernel(64, 40000.0 / 2822400.0);

void DsdProcessor::packDoP(const uint16_t* ch0Dsd, const uint16_t* ch1Dsd,
                           int32_t* outPcmInterleaved, size_t numFrames,
                           uint8_t& markerState) {
    for (size_t f = 0; f < numFrames; ++f) {
        uint8_t marker = (markerState == 0) ? kDopMarker0 : kDopMarker1;
        markerState = 1 - markerState;

        // Left channel: 8-bit marker in bits 16-23, 16 DSD bits in bits 0-15
        int32_t word0 = (static_cast<int32_t>(marker) << 16) | (ch0Dsd ? (ch0Dsd[f] & 0xFFFF) : 0);
        if (word0 & 0x00800000) word0 |= 0xFF000000; // Sign-extend 24-bit to 32-bit

        // Right channel
        int32_t word1 = (static_cast<int32_t>(marker) << 16) | (ch1Dsd ? (ch1Dsd[f] & 0xFFFF) : 0);
        if (word1 & 0x00800000) word1 |= 0xFF000000;

        outPcmInterleaved[f * 2 + 0] = word0;
        outPcmInterleaved[f * 2 + 1] = word1;
    }
}

void DsdProcessor::packDoPFloat(const uint16_t* ch0Dsd, const uint16_t* ch1Dsd,
                                float* outPcmInterleaved, size_t numFrames,
                                uint8_t& markerState) {
    constexpr float kInv24Bit = 1.0f / 8388608.0f; // 2^23
    for (size_t f = 0; f < numFrames; ++f) {
        uint8_t marker = (markerState == 0) ? kDopMarker0 : kDopMarker1;
        markerState = 1 - markerState;

        int32_t word0 = (static_cast<int32_t>(marker) << 16) | (ch0Dsd ? (ch0Dsd[f] & 0xFFFF) : 0);
        if (word0 & 0x00800000) word0 |= 0xFF000000;

        int32_t word1 = (static_cast<int32_t>(marker) << 16) | (ch1Dsd ? (ch1Dsd[f] & 0xFFFF) : 0);
        if (word1 & 0x00800000) word1 |= 0xFF000000;

        outPcmInterleaved[f * 2 + 0] = static_cast<float>(word0) * kInv24Bit;
        outPcmInterleaved[f * 2 + 1] = static_cast<float>(word1) * kInv24Bit;
    }
}

bool DsdProcessor::isDopStream(const int32_t* pcmInterleaved, size_t numFrames) {
    if (!pcmInterleaved || numFrames < 8) return false;

    // Check alternating markers across consecutive frames
    size_t validTransitions = 0;
    for (size_t f = 0; f < numFrames - 1; ++f) {
        uint8_t markerA = static_cast<uint8_t>((pcmInterleaved[f * 2 + 0] >> 16) & 0xFF);
        uint8_t markerB = static_cast<uint8_t>((pcmInterleaved[(f + 1) * 2 + 0] >> 16) & 0xFF);

        if ((markerA == kDopMarker0 && markerB == kDopMarker1) ||
            (markerA == kDopMarker1 && markerB == kDopMarker0)) {
            validTransitions++;
        }
    }

    return validTransitions >= (numFrames * 3) / 4;
}

bool DsdProcessor::isDopStreamFloat(const float* pcmInterleaved, size_t numFrames) {
    if (!pcmInterleaved || numFrames < 8) return false;

    constexpr float k24Bit = 8388608.0f;
    std::vector<int32_t> intPcm(std::min(numFrames, size_t(32)) * 2);
    for (size_t i = 0; i < intPcm.size(); ++i) {
        intPcm[i] = static_cast<int32_t>(std::clamp(pcmInterleaved[i], -1.0f, 1.0f) * k24Bit);
    }

    return isDopStream(intPcm.data(), intPcm.size() / 2);
}

bool DsdProcessor::unpackDoP(const int32_t* inPcmInterleaved,
                             uint16_t* outCh0Dsd, uint16_t* outCh1Dsd,
                             size_t numFrames) {
    if (!inPcmInterleaved || numFrames == 0) return false;

    for (size_t f = 0; f < numFrames; ++f) {
        int32_t word0 = inPcmInterleaved[f * 2 + 0];
        int32_t word1 = inPcmInterleaved[f * 2 + 1];

        if (outCh0Dsd) {
            outCh0Dsd[f] = static_cast<uint16_t>(word0 & 0xFFFF);
        }
        if (outCh1Dsd) {
            outCh1Dsd[f] = static_cast<uint16_t>(word1 & 0xFFFF);
        }
    }
    return true;
}

bool DsdProcessor::unpackDoPFloat(const float* inPcmInterleaved,
                                  uint16_t* outCh0Dsd, uint16_t* outCh1Dsd,
                                  size_t numFrames) {
    if (!inPcmInterleaved || numFrames == 0) return false;

    constexpr float k24Bit = 8388608.0f;
    for (size_t f = 0; f < numFrames; ++f) {
        int32_t word0 = static_cast<int32_t>(inPcmInterleaved[f * 2 + 0] * k24Bit);
        int32_t word1 = static_cast<int32_t>(inPcmInterleaved[f * 2 + 1] * k24Bit);

        if (outCh0Dsd) {
            outCh0Dsd[f] = static_cast<uint16_t>(word0 & 0xFFFF);
        }
        if (outCh1Dsd) {
            outCh1Dsd[f] = static_cast<uint16_t>(word1 & 0xFFFF);
        }
    }
    return true;
}

void DsdProcessor::decimateDsdToPcm(const uint8_t* dsdBytes, size_t numBytes,
                                    bool lsbFirst, float* outPcm, size_t& outSamples,
                                    size_t decimationFactor) {
    if (!dsdBytes || numBytes == 0 || !outPcm) {
        outSamples = 0;
        return;
    }

    size_t totalBits = numBytes * 8;
    outSamples = totalBits / decimationFactor;
    const size_t firLength = s_firKernel.size();

    // Standard +6dB headroom scaling for SACD reference level
    constexpr float kSacdGain = 2.0f;

    for (size_t s = 0; s < outSamples; ++s) {
        size_t centerBit = s * decimationFactor;
        float acc = 0.0f;

        for (size_t k = 0; k < firLength; ++k) {
            int64_t bitIdx = static_cast<int64_t>(centerBit) + static_cast<int64_t>(k) - static_cast<int64_t>(firLength / 2);
            if (bitIdx >= 0 && bitIdx < static_cast<int64_t>(totalBits)) {
                size_t byteIdx = static_cast<size_t>(bitIdx / 8);
                uint8_t bitPos = lsbFirst ? static_cast<uint8_t>(bitIdx % 8) : static_cast<uint8_t>(7 - (bitIdx % 8));
                bool bit = (dsdBytes[byteIdx] >> bitPos) & 1;
                // 1 -> +1.0f, 0 -> -1.0f
                float val = bit ? 1.0f : -1.0f;
                acc += val * s_firKernel[k];
            }
        }

        outPcm[s] = std::clamp(acc * kSacdGain, -1.0f, 1.0f);
    }
}

// Helper: read 32-bit big endian integer
static uint32_t readBe32(std::ifstream& f) {
    uint8_t b[4];
    f.read(reinterpret_cast<char*>(b), 4);
    return (static_cast<uint32_t>(b[0]) << 24) |
           (static_cast<uint32_t>(b[1]) << 16) |
           (static_cast<uint32_t>(b[2]) << 8) |
           static_cast<uint32_t>(b[3]);
}

// Helper: read 64-bit big endian integer
static uint64_t readBe64(std::ifstream& f) {
    uint8_t b[8];
    f.read(reinterpret_cast<char*>(b), 8);
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val = (val << 8) | b[i];
    }
    return val;
}

bool DsdParser::parseDsf(std::ifstream& file, DsdFileInfo& info) {
    // 1. DSD chunk (28 bytes)
    char magic[4];
    file.read(magic, 4);
    if (std::memcmp(magic, "DSD ", 4) != 0) return false;

    uint64_t dsdChunkSize = 0;
    uint64_t totalFileSize = 0;
    uint64_t metadataOffset = 0;
    file.read(reinterpret_cast<char*>(&dsdChunkSize), 8);
    file.read(reinterpret_cast<char*>(&totalFileSize), 8);
    file.read(reinterpret_cast<char*>(&metadataOffset), 8);

    // 2. fmt chunk (52 bytes)
    char fmtMagic[4];
    file.read(fmtMagic, 4);
    if (std::memcmp(fmtMagic, "fmt ", 4) != 0) return false;

    uint64_t fmtChunkSize = 0;
    uint32_t formatVersion = 0;
    uint32_t formatId = 0;
    uint32_t channelType = 0;
    uint32_t channelNum = 0;
    uint32_t samplingFrequency = 0;
    uint32_t bitsPerSample = 0;
    uint64_t sampleCount = 0;
    uint32_t blockSizePerChannel = 0;
    uint32_t reserved = 0;

    file.read(reinterpret_cast<char*>(&fmtChunkSize), 8);
    file.read(reinterpret_cast<char*>(&formatVersion), 4);
    file.read(reinterpret_cast<char*>(&formatId), 4);
    file.read(reinterpret_cast<char*>(&channelType), 4);
    file.read(reinterpret_cast<char*>(&channelNum), 4);
    file.read(reinterpret_cast<char*>(&samplingFrequency), 4);
    file.read(reinterpret_cast<char*>(&bitsPerSample), 4);
    file.read(reinterpret_cast<char*>(&sampleCount), 8);
    file.read(reinterpret_cast<char*>(&blockSizePerChannel), 4);
    file.read(reinterpret_cast<char*>(&reserved), 4);

    if (channelNum == 0 || samplingFrequency == 0 || sampleCount == 0) return false;

    // 3. data chunk
    char dataMagic[4];
    file.read(dataMagic, 4);
    if (std::memcmp(dataMagic, "data", 4) != 0) return false;

    uint64_t dataChunkSize = 0;
    file.read(reinterpret_cast<char*>(&dataChunkSize), 8);

    info.type = DsdFormatType::DSF;
    info.sampleRate = samplingFrequency;
    info.channels = static_cast<uint16_t>(channelNum);
    info.sampleCount = sampleCount;
    info.blockSize = blockSizePerChannel > 0 ? blockSizePerChannel : 4096;
    info.dataOffset = file.tellg();
    info.dataBytes = dataChunkSize >= 12 ? (dataChunkSize - 12) : 0;
    info.durationSeconds = static_cast<double>(sampleCount) / static_cast<double>(samplingFrequency);
    info.lsbFirst = true;

    return true;
}

bool DsdParser::parseDff(std::ifstream& file, DsdFileInfo& info) {
    char frm8[4];
    file.read(frm8, 4);
    if (std::memcmp(frm8, "FRM8", 4) != 0) return false;

    uint64_t frmSize = readBe64(file);
    (void)frmSize;

    char dsdMagic[4];
    file.read(dsdMagic, 4);
    if (std::memcmp(dsdMagic, "DSD ", 4) != 0) return false;

    bool foundFs = false;
    bool foundChnl = false;
    bool foundData = false;

    while (file.good()) {
        char chunkId[4];
        file.read(chunkId, 4);
        if (!file.good()) break;

        uint64_t chunkSize = readBe64(file);

        if (std::memcmp(chunkId, "FS  ", 4) == 0) {
            info.sampleRate = readBe32(file);
            foundFs = true;
            if (chunkSize > 4) file.seekg(chunkSize - 4, std::ios::cur);
        } else if (std::memcmp(chunkId, "CHNL", 4) == 0) {
            uint8_t chBytes[2];
            file.read(reinterpret_cast<char*>(chBytes), 2);
            info.channels = (static_cast<uint16_t>(chBytes[0]) << 8) | chBytes[1];
            foundChnl = true;
            if (chunkSize > 2) file.seekg(chunkSize - 2, std::ios::cur);
        } else if (std::memcmp(chunkId, "DSD ", 4) == 0) {
            info.dataOffset = file.tellg();
            info.dataBytes = chunkSize;
            foundData = true;
            break;
        } else if (std::memcmp(chunkId, "PROP", 4) == 0) {
            char propType[4];
            file.read(propType, 4);
            // SND subchunk continues immediately inside PROP
            continue;
        } else {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (foundFs && foundChnl && foundData && info.channels > 0 && info.sampleRate > 0) {
        info.type = DsdFormatType::DFF;
        info.sampleCount = (info.dataBytes * 8) / info.channels;
        info.durationSeconds = static_cast<double>(info.sampleCount) / static_cast<double>(info.sampleRate);
        info.blockSize = 1;
        info.lsbFirst = false;
        return true;
    }

    return false;
}

bool DsdParser::parseFile(const std::string& filePath, DsdFileInfo& info) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;

    char magic[4];
    file.read(magic, 4);
    file.seekg(0, std::ios::beg);

    if (std::memcmp(magic, "DSD ", 4) == 0) {
        return parseDsf(file, info);
    } else if (std::memcmp(magic, "FRM8", 4) == 0) {
        return parseDff(file, info);
    }

    return false;
}

} // namespace soniq::audio::dsd
