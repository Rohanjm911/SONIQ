#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace soniq::metadata {

class ArtworkExtractor {
public:
    static ArtworkExtractor& instance();

    // Sets the cache directory (defaults to AppData/SONIQ/artwork_cache)
    void setCacheDirectory(const std::string& cacheDir);
    [[nodiscard]] std::string cacheDirectory() const;

    // Checks if artwork is cached or extracts from audio file
    // Returns absolute file path to cached image, or empty string if no artwork exists
    std::string getOrExtractArtwork(const std::string& audioFilePath);

    // Direct extraction methods for specific formats
    static bool extractFlacPicture(const std::string& flacPath, const std::string& outImagePath);
    static bool extractId3v2Picture(const std::string& mp3Path, const std::string& outImagePath);
    static bool extractViaFfmpeg(const std::string& audioPath, const std::string& outImagePath);

private:
    ArtworkExtractor();
    ~ArtworkExtractor() = default;

    std::string generateCacheKey(const std::string& audioFilePath) const;

    std::string m_cacheDir;
};

} // namespace soniq::metadata
