#pragma once

#include "core/Types.hpp"
#include "core/AudioFormat.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <optional>

namespace soniq::audio::dsd {

enum class DsdFormatType {
    DSF,
    DFF,
    Unknown
};

enum class DsdRate {
    DSD64  = 2822400,
    DSD128 = 5644800,
    DSD256 = 11289600
};

enum class DsdPlaybackMode {
    DoP,            // DSD over PCM v1.1 (176.4 kHz 24-bit PCM container)
    DecimatedPcm    // Software decimation to 88.2 kHz / 176.4 kHz float PCM
};

struct DsdFileInfo {
    DsdFormatType type{DsdFormatType::Unknown};
    uint32_t sampleRate{2822400};
    uint16_t channels{2};
    uint64_t sampleCount{0};
    uint32_t blockSize{4096}; // DSF channel block size
    uint64_t dataOffset{0};
    uint64_t dataBytes{0};
    double durationSeconds{0.0};
    bool lsbFirst{true};       // DSF is LSB first, DFF is MSB first
};

class DsdProcessor {
public:
    static constexpr uint8_t kDopMarker0 = 0x05;
    static constexpr uint8_t kDopMarker1 = 0xFA;

    // DoP Framing & Packing (DSD v1.1 Standard)
    // Packs 16 DSD bits per channel into 24-bit PCM (stored in int32_t or float32)
    static void packDoP(const uint16_t* ch0Dsd, const uint16_t* ch1Dsd,
                        int32_t* outPcmInterleaved, size_t numFrames,
                        uint8_t& markerState);

    static void packDoPFloat(const uint16_t* ch0Dsd, const uint16_t* ch1Dsd,
                             float* outPcmInterleaved, size_t numFrames,
                             uint8_t& markerState);

    // DoP Detection & Unpacking
    // Verifies 0x05 / 0xFA alternating markers and extracts 16-bit DSD words
    static bool isDopStream(const int32_t* pcmInterleaved, size_t numFrames);
    static bool isDopStreamFloat(const float* pcmInterleaved, size_t numFrames);

    static bool unpackDoP(const int32_t* inPcmInterleaved,
                          uint16_t* outCh0Dsd, uint16_t* outCh1Dsd,
                          size_t numFrames);

    static bool unpackDoPFloat(const float* inPcmInterleaved,
                               uint16_t* outCh0Dsd, uint16_t* outCh1Dsd,
                               size_t numFrames);

    // DSD-to-PCM Decimation
    // Decimates 1-bit DSD bitstream to 32-bit float PCM at 88.2 kHz or 176.4 kHz
    // with 64-tap linear-phase FIR low-pass filter (40 kHz cutoff)
    static void decimateDsdToPcm(const uint8_t* dsdBytes, size_t numBytes,
                                 bool lsbFirst, float* outPcm, size_t& outSamples,
                                 size_t decimationFactor = 32);
};

// DSF and DFF File Parser
class DsdParser {
public:
    static bool parseFile(const std::string& filePath, DsdFileInfo& info);
    static bool parseDsf(std::ifstream& file, DsdFileInfo& info);
    static bool parseDff(std::ifstream& file, DsdFileInfo& info);
};

} // namespace soniq::audio::dsd
