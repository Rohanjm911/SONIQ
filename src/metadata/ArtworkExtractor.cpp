#include "ArtworkExtractor.hpp"
#include "core/Logger.hpp"
#include <QStandardPaths>
#include <QDir>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace soniq::metadata {

ArtworkExtractor& ArtworkExtractor::instance() {
    static ArtworkExtractor s_instance;
    return s_instance;
}

ArtworkExtractor::ArtworkExtractor() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString cacheFolder = appData + "/artwork_cache";
    QDir().mkpath(cacheFolder);
    m_cacheDir = cacheFolder.toStdString();
}

void ArtworkExtractor::setCacheDirectory(const std::string& cacheDir) {
    m_cacheDir = cacheDir;
    std::filesystem::create_directories(m_cacheDir);
}

std::string ArtworkExtractor::cacheDirectory() const {
    return m_cacheDir;
}

std::string ArtworkExtractor::generateCacheKey(const std::string& audioFilePath) const {
    // Combine file path and file size to generate a unique cache key
    std::hash<std::string> hasher;
    size_t hashVal = hasher(audioFilePath);

    std::error_code ec;
    uintmax_t sz = std::filesystem::file_size(audioFilePath, ec);
    hashVal ^= std::hash<uintmax_t>{}(sz) + 0x9e3779b9 + (hashVal << 6) + (hashVal >> 2);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hashVal;
    return ss.str();
}

bool ArtworkExtractor::extractFlacPicture(const std::string& flacPath, const std::string& outImagePath) {
    std::ifstream file(flacPath, std::ios::binary);
    if (!file.is_open()) return false;

    char magic[4];
    file.read(magic, 4);
    if (std::memcmp(magic, "fLaC", 4) != 0) return false;

    bool isLast = false;
    while (!isLast && file.good()) {
        uint8_t header[4];
        file.read(reinterpret_cast<char*>(header), 4);
        if (!file.good()) break;

        isLast = (header[0] & 0x80) != 0;
        uint8_t blockType = header[0] & 0x7F;
        uint32_t blockSize = (static_cast<uint32_t>(header[1]) << 16) |
                             (static_cast<uint32_t>(header[2]) << 8) |
                             static_cast<uint32_t>(header[3]);

        if (blockType == 6) { // METADATA_BLOCK_PICTURE
            // Format:
            // 4 bytes: picture type
            // 4 bytes: MIME type length
            // N bytes: MIME type
            // 4 bytes: description length
            // M bytes: description
            // 4 bytes: width, 4 bytes: height, 4 bytes: color depth, 4 bytes: indexed colors
            // 4 bytes: picture data length
            // L bytes: picture data
            uint32_t picType = 0;
            file.read(reinterpret_cast<char*>(&picType), 4);

            uint8_t mimeLenBytes[4];
            file.read(reinterpret_cast<char*>(mimeLenBytes), 4);
            uint32_t mimeLen = (static_cast<uint32_t>(mimeLenBytes[0]) << 24) |
                               (static_cast<uint32_t>(mimeLenBytes[1]) << 16) |
                               (static_cast<uint32_t>(mimeLenBytes[2]) << 8) |
                               static_cast<uint32_t>(mimeLenBytes[3]);

            if (mimeLen > 128) return false;
            file.seekg(mimeLen, std::ios::cur);

            uint8_t descLenBytes[4];
            file.read(reinterpret_cast<char*>(descLenBytes), 4);
            uint32_t descLen = (static_cast<uint32_t>(descLenBytes[0]) << 24) |
                               (static_cast<uint32_t>(descLenBytes[1]) << 16) |
                               (static_cast<uint32_t>(descLenBytes[2]) << 8) |
                               static_cast<uint32_t>(descLenBytes[3]);

            if (descLen > 1024) return false;
            file.seekg(descLen, std::ios::cur);

            // Skip width, height, depth, colors (16 bytes)
            file.seekg(16, std::ios::cur);

            uint8_t dataLenBytes[4];
            file.read(reinterpret_cast<char*>(dataLenBytes), 4);
            uint32_t dataLen = (static_cast<uint32_t>(dataLenBytes[0]) << 24) |
                               (static_cast<uint32_t>(dataLenBytes[1]) << 16) |
                               (static_cast<uint32_t>(dataLenBytes[2]) << 8) |
                               static_cast<uint32_t>(dataLenBytes[3]);

            if (dataLen > 0 && dataLen < 50 * 1024 * 1024) { // safety limit 50MB
                std::vector<char> picData(dataLen);
                file.read(picData.data(), dataLen);

                std::ofstream out(outImagePath, std::ios::binary);
                if (out.is_open()) {
                    out.write(picData.data(), dataLen);
                    out.close();
                    return true;
                }
            }
            return false;
        } else {
            file.seekg(blockSize, std::ios::cur);
        }
    }
    return false;
}

bool ArtworkExtractor::extractId3v2Picture(const std::string& mp3Path, const std::string& outImagePath) {
    std::ifstream file(mp3Path, std::ios::binary);
    if (!file.is_open()) return false;

    char tag[3];
    file.read(tag, 3);
    if (std::memcmp(tag, "ID3", 3) != 0) return false;

    uint8_t version[2];
    file.read(reinterpret_cast<char*>(version), 2);
    uint8_t flags = 0;
    file.read(reinterpret_cast<char*>(&flags), 1);

    uint8_t sizeBytes[4];
    file.read(reinterpret_cast<char*>(sizeBytes), 4);
    // Synchsafe integer
    uint32_t tagSize = ((sizeBytes[0] & 0x7F) << 21) |
                       ((sizeBytes[1] & 0x7F) << 14) |
                       ((sizeBytes[2] & 0x7F) << 7) |
                       (sizeBytes[3] & 0x7F);

    uint32_t bytesRead = 0;
    while (bytesRead < tagSize && file.good()) {
        char frameId[4];
        file.read(frameId, 4);
        bytesRead += 4;
        if (!file.good() || frameId[0] == 0) break;

        uint8_t frameSizeBytes[4];
        file.read(reinterpret_cast<char*>(frameSizeBytes), 4);
        bytesRead += 4;

        uint32_t frameSize = 0;
        if (version[0] == 4) { // ID3v2.4 synchsafe
            frameSize = ((frameSizeBytes[0] & 0x7F) << 21) |
                        ((frameSizeBytes[1] & 0x7F) << 14) |
                        ((frameSizeBytes[2] & 0x7F) << 7) |
                        (frameSizeBytes[3] & 0x7F);
        } else {
            frameSize = (static_cast<uint32_t>(frameSizeBytes[0]) << 24) |
                        (static_cast<uint32_t>(frameSizeBytes[1]) << 16) |
                        (static_cast<uint32_t>(frameSizeBytes[2]) << 8) |
                        static_cast<uint32_t>(frameSizeBytes[3]);
        }

        uint8_t frameFlags[2];
        file.read(reinterpret_cast<char*>(frameFlags), 2);
        bytesRead += 2;

        if (std::memcmp(frameId, "APIC", 4) == 0 && frameSize > 10 && frameSize < 50 * 1024 * 1024) {
            std::vector<char> frameData(frameSize);
            file.read(frameData.data(), frameSize);

            // APIC payload:
            // 1 byte text encoding
            // MIME type (null terminated)
            // 1 byte picture type
            // Description (null terminated)
            // Binary image data
            size_t pos = 1;
            while (pos < frameSize && frameData[pos] != 0) pos++;
            pos++; // skip null
            pos++; // skip picture type
            while (pos < frameSize && frameData[pos] != 0) pos++;
            pos++; // skip description null

            if (pos < frameSize) {
                size_t imageBytes = frameSize - pos;
                std::ofstream out(outImagePath, std::ios::binary);
                if (out.is_open()) {
                    out.write(frameData.data() + pos, imageBytes);
                    out.close();
                    return true;
                }
            }
            return false;
        } else {
            file.seekg(frameSize, std::ios::cur);
            bytesRead += frameSize;
        }
    }
    return false;
}

bool ArtworkExtractor::extractViaFfmpeg(const std::string& audioPath, const std::string& outImagePath) {
    namespace fs = std::filesystem;
    std::string ffmpegExe = "C:\\ffmpeg\\ffmpeg.exe";
    std::error_code ec;
    if (!fs::exists(ffmpegExe, ec)) {
        ffmpegExe = "ffmpeg";
    }

    std::string cmd = std::format("\"{}\" -v quiet -y -i \"{}\" -an -vcodec copy \"{}\"",
                                  ffmpegExe, audioPath, outImagePath);
#if defined(_WIN32)
    int ret = _wsystem(std::wstring(cmd.begin(), cmd.end()).c_str());
#else
    int ret = system(cmd.c_str());
#endif

    if (ret == 0 && fs::exists(outImagePath, ec) && fs::file_size(outImagePath, ec) > 100) {
        return true;
    }
    return false;
}

std::string ArtworkExtractor::getOrExtractArtwork(const std::string& audioFilePath) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(audioFilePath, ec)) return "";

    std::string key = generateCacheKey(audioFilePath);
    std::string cachedPath = m_cacheDir + "/" + key + ".jpg";

    if (fs::exists(cachedPath, ec) && fs::file_size(cachedPath, ec) > 100) {
        return cachedPath;
    }

    // Try format-specific direct extraction first
    std::string ext = fs::path(audioFilePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    bool ok = false;
    if (ext == ".flac") {
        ok = extractFlacPicture(audioFilePath, cachedPath);
    } else if (ext == ".mp3") {
        ok = extractId3v2Picture(audioFilePath, cachedPath);
    }

    // Universal fallback: FFmpeg cover extraction
    if (!ok) {
        ok = extractViaFfmpeg(audioFilePath, cachedPath);
    }

    if (ok && fs::exists(cachedPath, ec)) {
        return cachedPath;
    }

    return "";
}

} // namespace soniq::metadata
