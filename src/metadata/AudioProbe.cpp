#include "AudioProbe.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <array>
#include <format>
#include <QProcess>
#include <QFile>
#include <QFileInfo>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace soniq::metadata {

std::vector<std::string> AudioProbe::supportedExtensions() {
    return {
        ".flac", ".mp3", ".wav", ".m4a", ".aac", ".alac", ".ogg",
        ".opus", ".aiff", ".aif", ".ape", ".wv", ".dsf", ".dff",
        ".wma", ".tta", ".tak", ".ac3", ".dts"
    };
}

bool AudioProbe::isFormatSupported(const std::string& extensionOrCodec) noexcept {
    std::string lower = extensionOrCodec;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (!lower.empty() && lower[0] != '.') lower = "." + lower;

    const auto& exts = supportedExtensions();
    return std::find(exts.begin(), exts.end(), lower) != exts.end();
}

static bool probeWavHeader(const std::string& filePath, AudioInspectionResult& res) {
    namespace fs = std::filesystem;
    fs::path p(QString::fromStdString(filePath).toStdWString());
    std::ifstream f(p, std::ios::binary);
    if (!f.is_open()) return false;

    char riff[4];
    f.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) return false;

    uint32_t fileSize = 0;
    f.read(reinterpret_cast<char*>(&fileSize), 4);

    char wave[4];
    f.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) return false;

    while (f.good()) {
        char chunkId[4];
        uint32_t chunkSize = 0;
        f.read(chunkId, 4);
        f.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (!f.good()) break;

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            uint16_t audioFormat = 0;
            uint16_t numChannels = 0;
            uint32_t sampleRate = 0;
            uint32_t byteRate = 0;
            uint16_t blockAlign = 0;
            uint16_t bitsPerSample = 0;

            f.read(reinterpret_cast<char*>(&audioFormat), 2);
            f.read(reinterpret_cast<char*>(&numChannels), 2);
            f.read(reinterpret_cast<char*>(&sampleRate), 4);
            f.read(reinterpret_cast<char*>(&byteRate), 4);
            f.read(reinterpret_cast<char*>(&blockAlign), 2);
            f.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            core::SampleFormat sFmt = core::SampleFormat::Float32;
            if (audioFormat == 1) { // PCM Int
                if (bitsPerSample == 16) sFmt = core::SampleFormat::Int16;
                else if (bitsPerSample == 24) sFmt = core::SampleFormat::Int24;
                else if (bitsPerSample == 32) sFmt = core::SampleFormat::Int32;
            } else if (audioFormat == 3) { // IEEE Float
                sFmt = core::SampleFormat::Float32;
            }

            core::ChannelLayout layout = (numChannels == 1) ? core::ChannelLayout::Mono : core::ChannelLayout::Stereo;
            uint32_t bitrate = byteRate * 8 / 1000;

            res.format = core::AudioFormat(sampleRate, numChannels, bitsPerSample, sFmt, layout, "PCM", ".wav", bitrate);
            res.isLossless = true;
            res.isHiRes = (sampleRate > 48000 || bitsPerSample > 16);
            res.containerFormat = "WAV / RIFF";
            res.codecLongName = (audioFormat == 3) ? "PCM IEEE Floating-Point" : "PCM Uncompressed Integer";
            res.bitRate = byteRate * 8;

            if (chunkSize > 16) {
                f.seekg(chunkSize - 16, std::ios::cur);
            }
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            if (res.format.sampleRate() > 0 && res.format.channels() > 0 && res.format.bitDepth() > 0) {
                uint32_t bytesPerSec = res.format.sampleRate() * res.format.channels() * (res.format.bitDepth() / 8);
                if (bytesPerSec > 0) {
                    res.metadata.durationSeconds = static_cast<double>(chunkSize) / bytesPerSec;
                }
            }
            break;
        } else {
            f.seekg(chunkSize, std::ios::cur);
        }
    }

    res.success = res.format.isValid();
    return res.success;
}

AudioInspectionResult AudioProbe::probeFile(const std::string& filePath) {
    AudioInspectionResult result;
    result.filePath = filePath;

    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path fsPath(QString::fromStdString(filePath).toStdWString());
    if (!fs::exists(fsPath, ec) || !fs::is_regular_file(fsPath, ec)) {
        result.warnings.push_back("File does not exist or is not a regular file");
        return result;
    }

    result.metadata.fileSize = fs::file_size(fsPath, ec);
    std::string ext = fsPath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    QFileInfo fileInfo(QString::fromStdString(filePath));

    // Fast-path: Header inspection for WAV
    if (ext == ".wav" && probeWavHeader(filePath, result)) {
        result.metadata.title = fileInfo.baseName().toStdString();
        result.isBitPerfectEligible = result.isLossless;
        result.bitPerfectAssessment = result.isLossless
            ? "Eligible: Uncompressed lossless PCM format. Can be delivered bit-perfect via Exclusive WASAPI."
            : "Ineligible: Lossy container.";
        result.technicalSummary = std::format("{} / {} / {:.1f} kHz / {}-bit",
            result.containerFormat, result.codecLongName,
            result.format.sampleRate() / 1000.0, result.format.bitDepth());
        return result;
    }

    // FFprobe execution for comprehensive metadata and all other formats
    QString ffprobeCmd = "C:/ffmpeg/ffprobe.exe";
    if (!QFile::exists(ffprobeCmd)) {
        ffprobeCmd = "ffprobe";
    }

    QProcess probeProc;
    probeProc.start(ffprobeCmd, QStringList{
        "-v", "quiet",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        QString::fromStdString(filePath)
    });

    std::string jsonOutput;
    if (probeProc.waitForFinished(5000) && probeProc.exitCode() == 0) {
        jsonOutput = probeProc.readAllStandardOutput().toStdString();
    }

    if (!jsonOutput.empty()) {
        auto extractStr = [&](std::string_view key) -> std::string {
            std::string pattern = std::format("\"{}\": \"", key);
            size_t pos = jsonOutput.find(pattern);
            if (pos == std::string::npos) return "";
            pos += pattern.length();
            size_t endPos = jsonOutput.find('"', pos);
            if (endPos == std::string::npos) return "";
            return jsonOutput.substr(pos, endPos - pos);
        };

        auto extractNum = [&](std::string_view key) -> double {
            std::string pattern = std::format("\"{}\": ", key);
            size_t pos = jsonOutput.find(pattern);
            if (pos == std::string::npos) return 0.0;
            pos += pattern.length();
            if (pos < jsonOutput.size() && jsonOutput[pos] == '"') pos++;
            try {
                return std::stod(jsonOutput.substr(pos));
            } catch (...) {
                return 0.0;
            }
        };

        std::string codecName = extractStr("codec_name");
        std::string codecType = extractStr("codec_type");
        std::string sampleFmt = extractStr("sample_fmt");
        uint32_t sampleRate = static_cast<uint32_t>(extractNum("sample_rate"));
        uint16_t channels = static_cast<uint16_t>(extractNum("channels"));
        uint16_t bitDepth = static_cast<uint16_t>(extractNum("bits_per_raw_sample"));
        if (bitDepth == 0) bitDepth = static_cast<uint16_t>(extractNum("bits_per_sample"));
        if (bitDepth == 0) {
            if (codecName == "flac" || codecName == "alac") bitDepth = 16;
            else if (sampleFmt.find("s16") != std::string::npos) bitDepth = 16;
            else if (sampleFmt.find("s32") != std::string::npos || sampleFmt.find("flt") != std::string::npos) bitDepth = 24;
            else bitDepth = 16;
        }

        double duration = extractNum("duration");
        uint64_t bitRate = static_cast<uint64_t>(extractNum("bit_rate"));

        std::string title = extractStr("title");
        std::string artist = extractStr("artist");
        std::string album = extractStr("album");
        std::string genre = extractStr("genre");
        std::string date = extractStr("date");

        if (title.empty()) title = fileInfo.baseName().toStdString();

        uint32_t year = 0;
        try {
            if (!date.empty()) year = std::stoul(date.substr(0, 4));
        } catch (...) {}

        result.metadata.title = title;
        result.metadata.artist = artist.empty() ? "Unknown Artist" : artist;
        result.metadata.album = album.empty() ? "Unknown Album" : album;
        result.metadata.genre = genre.empty() ? "Unknown" : genre;
        result.metadata.year = year;
        result.metadata.durationSeconds = duration;

        result.format = core::AudioFormat(sampleRate > 0 ? sampleRate : 44100,
                                          channels > 0 ? channels : 2,
                                          bitDepth,
                                          core::SampleFormat::Float32,
                                          channels == 1 ? core::ChannelLayout::Mono : core::ChannelLayout::Stereo,
                                          codecName.empty() ? "PCM" : codecName,
                                          ext,
                                          static_cast<uint32_t>(bitRate / 1000));

        result.containerFormat = extractStr("format_long_name");
        if (result.containerFormat.empty()) result.containerFormat = ext;
        result.codecLongName = extractStr("codec_long_name");
        result.bitRate = bitRate;
        result.isLossless = result.format.isLossless();
        result.isHiRes = result.format.isHiRes();
        result.isBitPerfectEligible = result.isLossless;

        result.bitPerfectAssessment = result.isLossless
            ? "Bit-perfect eligible: Source is a lossless audio format. Exact decoded PCM samples can be rendered without alterations."
            : "Lossy compressed source (MP3/AAC/Opus): Decoding reconstructs an approximation of the original signal. Bit-perfect preservation applies to the decoded PCM stream only.";

        result.technicalSummary = std::format("{} [{}] - {:.1f} kHz, {}-bit, {}",
            result.format.codec(), result.format.container(),
            result.format.sampleRate() / 1000.0, result.format.bitDepth(),
            result.format.channels() == 1 ? "Mono" : "Stereo");

        result.success = true;
    } else {
        // Fallback default for any discovered audio file
        result.metadata.title = fileInfo.baseName().toStdString();
        result.metadata.artist = "Unknown Artist";
        result.metadata.album = "Unknown Album";
        result.format = core::AudioFormat(44100, 2, 16, core::SampleFormat::Float32,
                                          core::ChannelLayout::Stereo, ext.empty() ? "UNKNOWN" : ext.substr(1), ext);
        result.technicalSummary = "Format probed via filename fallback";
        result.success = true;
    }

    return result;
}

} // namespace soniq::metadata
