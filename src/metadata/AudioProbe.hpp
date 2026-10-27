#pragma once

#include "core/Types.hpp"
#include "core/AudioFormat.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace soniq::metadata {

struct TrackMetadata {
    std::string title;
    std::string artist;
    std::string album;
    std::string albumArtist;
    std::string genre;
    std::string composer;
    uint32_t year{0};
    uint32_t trackNumber{0};
    uint32_t totalTracks{0};
    uint32_t discNumber{1};
    uint32_t totalDiscs{1};
    double durationSeconds{0.0};
    uint64_t fileSize{0};
    std::string lyrics;
    bool hasEmbeddedArtwork{false};
    std::unordered_map<std::string, std::string> extraTags;
};

struct AudioInspectionResult {
    bool success{false};
    std::string filePath;
    core::AudioFormat format;
    TrackMetadata metadata;
    std::string containerFormat;
    std::string codecLongName;
    std::string profile;
    uint64_t bitRate{0};
    bool isLossless{false};
    bool isHiRes{false};
    bool isBitPerfectEligible{false};
    std::string bitPerfectAssessment;
    std::string technicalSummary;
    std::vector<std::string> warnings;
};

class AudioProbe {
public:
    static AudioInspectionResult probeFile(const std::string& filePath);
    static bool isFormatSupported(const std::string& extensionOrCodec) noexcept;
    static std::vector<std::string> supportedExtensions();
};

} // namespace soniq::metadata
