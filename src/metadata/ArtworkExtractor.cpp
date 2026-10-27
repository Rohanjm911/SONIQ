#include "ArtworkExtractor.hpp"
#include "core/Logger.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
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
    std::hash<std::string> hasher;
    size_t hashVal = hasher(audioFilePath);

    std::error_code ec;
    std::filesystem::path p(QString::fromStdString(audioFilePath).toStdWString());
    uintmax_t sz = std::filesystem::file_size(p, ec);
    hashVal ^= std::hash<uintmax_t>{}(sz) + 0x9e3779b9 + (hashVal << 6) + (hashVal >> 2);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hashVal;
    return ss.str();
}

bool ArtworkExtractor::extractFlacPicture(const std::string& flacPath, const std::string& outImagePath) {
    namespace fs = std::filesystem;
    fs::path p(QString::fromStdString(flacPath).toStdWString());
    std::ifstream file(p, std::ios::binary);
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
    namespace fs = std::filesystem;
    fs::path p(QString::fromStdString(mp3Path).toStdWString());
    std::ifstream file(p, std::ios::binary);
    if (!file.is_open()) return false;

    char header[10];
    file.read(header, 10);
    if (std::memcmp(header, "ID3", 3) != 0) return false;

    // Synchsafe integer decoder for ID3v2 total tag size
    uint32_t tagSize = ((header[6] & 0x7F) << 21) |
                       ((header[7] & 0x7F) << 14) |
                       ((header[8] & 0x7F) << 7)  |
                       (header[9] & 0x7F);

    size_t bytesRead = 0;
    while (bytesRead < tagSize && file.good()) {
        char frameHeader[10];
        file.read(frameHeader, 10);
        if (!file.good()) break;
        bytesRead += 10;

        // Check for padding (null frame ID)
        if (frameHeader[0] == 0) break;

        std::string frameId(frameHeader, 4);
        uint32_t frameSize = (static_cast<uint8_t>(frameHeader[4]) << 24) |
                             (static_cast<uint8_t>(frameHeader[5]) << 16) |
                             (static_cast<uint8_t>(frameHeader[6]) << 8)  |
                             static_cast<uint8_t>(frameHeader[7]);

        if (frameSize == 0 || frameSize > 50 * 1024 * 1024) break;

        if (frameId == "APIC") { // Attached picture
            std::vector<char> frameData(frameSize);
            file.read(frameData.data(), frameSize);
            bytesRead += frameSize;

            if (frameSize < 5) return false;

            size_t pos = 1; // skip encoding byte
            while (pos < frameSize && frameData[pos] != 0) pos++; // skip MIME
            pos++; // skip null terminator
            if (pos >= frameSize) return false;

            pos++; // skip picture type (1 byte)
            while (pos < frameSize && frameData[pos] != 0) pos++; // skip description
            pos++; // skip null terminator
            if (pos >= frameSize) return false;

            // Raw image bytes remain
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
    QString ffmpegExe = "C:/ffmpeg/ffmpeg.exe";
    if (!QFile::exists(ffmpegExe)) {
        ffmpegExe = "ffmpeg";
    }

    QProcess proc;
    proc.start(ffmpegExe, QStringList{
        "-v", "quiet",
        "-y",
        "-i", QString::fromStdString(audioPath),
        "-an",
        "-vcodec", "copy",
        QString::fromStdString(outImagePath)
    });

    if (proc.waitForFinished(5000) && proc.exitCode() == 0) {
        QFileInfo fi(QString::fromStdString(outImagePath));
        if (fi.exists() && fi.size() > 100) {
            return true;
        }
    }
    return false;
}

std::string ArtworkExtractor::getOrExtractArtwork(const std::string& audioFilePath) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path p(QString::fromStdString(audioFilePath).toStdWString());
    if (!fs::exists(p, ec)) return "";

    std::string key = generateCacheKey(audioFilePath);
    std::string cachedPath = m_cacheDir + "/" + key + ".jpg";

    if (fs::exists(cachedPath, ec) && fs::file_size(cachedPath, ec) > 100) {
        return cachedPath;
    }

    // Try format-specific direct extraction first
    std::string ext = p.extension().string();
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
