#include "FFmpegAudioDecoder.hpp"
#include "core/Logger.hpp"
#include <filesystem>
#include <format>
#include <algorithm>
#include <cmath>
#include <vector>
#include <QString>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

namespace soniq::audio {

FFmpegAudioDecoder::FFmpegAudioDecoder() = default;

FFmpegAudioDecoder::~FFmpegAudioDecoder() {
    close();
}

void FFmpegAudioDecoder::closePipe() {
#if defined(_WIN32)
    if (m_pipeHandle) {
        fclose(m_pipeHandle);
        m_pipeHandle = nullptr;
    }
    if (m_processHandle) {
        HANDLE hProcess = reinterpret_cast<HANDLE>(m_processHandle);
        DWORD exitCode = 0;
        if (GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE) {
            TerminateProcess(hProcess, 0);
            WaitForSingleObject(hProcess, 500);
        }
        CloseHandle(hProcess);
        m_processHandle = nullptr;
    }
#else
    if (m_pipeHandle) {
        pclose(m_pipeHandle);
        m_pipeHandle = nullptr;
    }
#endif
}

void FFmpegAudioDecoder::close() {
    closePipe();
    if (m_wavFile.is_open()) {
        m_wavFile.close();
    }
    m_isOpen = false;
    m_eof = false;
    m_currentFrame = 0;
    m_durationSeconds = 0.0;
    m_isWavDirect = false;
}

bool FFmpegAudioDecoder::openWavDirect(const std::string& filePath) {
    std::filesystem::path fsPath(QString::fromStdString(filePath).toStdWString());
    m_wavFile.open(fsPath, std::ios::binary);
    if (!m_wavFile.is_open()) return false;

    char riff[4];
    m_wavFile.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) return false;

    uint32_t fileSize = 0;
    m_wavFile.read(reinterpret_cast<char*>(&fileSize), 4);

    char wave[4];
    m_wavFile.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) return false;

    bool foundFmt = false;
    bool foundData = false;

    while (m_wavFile.good()) {
        char chunkId[4];
        uint32_t chunkSize = 0;
        m_wavFile.read(chunkId, 4);
        m_wavFile.read(reinterpret_cast<char*>(&chunkSize), 4);
        if (!m_wavFile.good()) break;

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            uint16_t audioFormat = 0;
            uint16_t numChannels = 0;
            uint32_t sampleRate = 0;
            uint32_t byteRate = 0;
            uint16_t blockAlign = 0;
            uint16_t bitsPerSample = 0;

            m_wavFile.read(reinterpret_cast<char*>(&audioFormat), 2);
            m_wavFile.read(reinterpret_cast<char*>(&numChannels), 2);
            m_wavFile.read(reinterpret_cast<char*>(&sampleRate), 4);
            m_wavFile.read(reinterpret_cast<char*>(&byteRate), 4);
            m_wavFile.read(reinterpret_cast<char*>(&blockAlign), 2);
            m_wavFile.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            m_wavAudioFormat = audioFormat;
            m_wavBitsPerSample = bitsPerSample;

            core::SampleFormat sFmt = core::SampleFormat::Float32;
            if (audioFormat == 1) {
                if (bitsPerSample == 16) sFmt = core::SampleFormat::Int16;
                else if (bitsPerSample == 24) sFmt = core::SampleFormat::Int24;
                else if (bitsPerSample == 32) sFmt = core::SampleFormat::Int32;
            } else if (audioFormat == 3) {
                sFmt = core::SampleFormat::Float32;
            }

            m_format = core::AudioFormat(sampleRate, numChannels, bitsPerSample, sFmt,
                                         numChannels == 1 ? core::ChannelLayout::Mono : core::ChannelLayout::Stereo,
                                         "PCM", "WAV", (byteRate * 8) / 1000);
            foundFmt = true;

            if (chunkSize > 16) {
                m_wavFile.seekg(chunkSize - 16, std::ios::cur);
            }
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            m_wavDataStart = m_wavFile.tellg();
            m_wavDataBytes = chunkSize;
            foundData = true;
            break;
        } else {
            m_wavFile.seekg(chunkSize, std::ios::cur);
        }
    }

    if (foundFmt && foundData && m_format.bytesPerSecond() > 0) {
        m_durationSeconds = static_cast<double>(m_wavDataBytes) / static_cast<double>(m_format.bytesPerSecond());
        m_isWavDirect = true;
        m_currentFrame = 0;
        return true;
    }

    return false;
}

bool FFmpegAudioDecoder::openFfmpegPipe(const std::string& filePath, double startSeconds) {
    closePipe();

    namespace fs = std::filesystem;
    std::string ffmpegExe = "C:\\ffmpeg\\ffmpeg.exe";
    std::error_code ec;
    if (!fs::exists(ffmpegExe, ec)) {
        ffmpegExe = "ffmpeg";
    }

    // Seek argument before input for fast keyframe/demuxer seeking
    std::string seekArg = (startSeconds > 0.001) ? std::format("-ss {:.3f} ", startSeconds) : "";

#if defined(_WIN32)
    std::wstring wExe = QString::fromStdString(ffmpegExe).toStdWString();
    std::wstring wSeek = QString::fromStdString(seekArg).toStdWString();
    std::wstring wPath = QString::fromStdString(filePath).toStdWString();

    std::wstring cmdLine = L"\"" + wExe + L"\" -v quiet -hide_banner " + wSeek + L"-i \"" + wPath + L"\" -f f32le pipe:1";

    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        SONIQ_LOG_ERROR("CreatePipe failed for FFmpeg decoder: error {}", GetLastError());
        return false;
    }

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hWritePipe;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdBuffer(cmdLine.begin(), cmdLine.end());
    cmdBuffer.push_back(L'\0');

    BOOL success = CreateProcessW(
        NULL,
        cmdBuffer.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    CloseHandle(hWritePipe);

    if (!success) {
        SONIQ_LOG_ERROR("CreateProcessW failed for FFmpeg: error {}", GetLastError());
        CloseHandle(hReadPipe);
        return false;
    }

    CloseHandle(pi.hThread);
    m_processHandle = pi.hProcess;

    int fd = _open_osfhandle(reinterpret_cast<intptr_t>(hReadPipe), _O_RDONLY | _O_BINARY);
    if (fd == -1) {
        SONIQ_LOG_ERROR("_open_osfhandle failed for FFmpeg read pipe");
        CloseHandle(hReadPipe);
        return false;
    }

    m_pipeHandle = _fdopen(fd, "rb");
    if (!m_pipeHandle) {
        SONIQ_LOG_ERROR("_fdopen failed for FFmpeg read pipe");
        _close(fd);
        return false;
    }
#else
    // Force decode into raw 32-bit float IEEE little-endian PCM on stdout pipe
    std::string cmd = std::format("\"{}\" -v quiet -hide_banner {} -i \"{}\" -f f32le pipe:1",
                                  ffmpegExe, seekArg, filePath);
    m_pipeHandle = popen(cmd.c_str(), "r");
#endif

    if (!m_pipeHandle) {
        SONIQ_LOG_ERROR("Failed to spawn FFmpeg decoding pipe for: {}", filePath);
        return false;
    }

    m_currentFrame = static_cast<uint64_t>(startSeconds * static_cast<double>(m_format.sampleRate()));
    return true;
}

core::Result<void> FFmpegAudioDecoder::open(const std::string& filePath) {
    close();
    m_filePath = filePath;

    // 1. Probe stream attributes and metadata first
    auto inspection = metadata::AudioProbe::probeFile(filePath);
    if (!inspection.success) {
        return core::Error::make(-1, "Unable to probe audio file", "FFmpegDecoder");
    }

    m_format = inspection.format;
    m_durationSeconds = inspection.metadata.durationSeconds;

    // 2. Fast-path check: direct uncompressed WAV
    namespace fs = std::filesystem;
    std::string ext = fs::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".wav" && openWavDirect(filePath)) {
        m_isOpen = true;
        m_eof = false;
        SONIQ_LOG_INFO("Opened WAV file directly: {} ({:.1f} kHz, {}-bit)",
                       filePath, m_format.sampleRate() / 1000.0, m_format.bitDepth());
        return core::Result<void>::ok();
    }

    // 3. Universal path: FFmpeg decoding stream
    if (!openFfmpegPipe(filePath, 0.0)) {
        return core::Error::make(-2, "Failed to initialize FFmpeg decoding stream", "FFmpegDecoder");
    }

    m_isOpen = true;
    m_eof = false;
    SONIQ_LOG_INFO("Opened audio stream via FFmpeg: {} ({:.1f} kHz, {}-bit, codec: {})",
                   filePath, m_format.sampleRate() / 1000.0, m_format.bitDepth(), m_format.codec());
    return core::Result<void>::ok();
}

size_t FFmpegAudioDecoder::readFrames(float* buffer, size_t frames) {
    if (!m_isOpen || m_eof || buffer == nullptr || frames == 0) return 0;

    const uint16_t channels = m_format.channels() > 0 ? m_format.channels() : 2;

    if (m_isWavDirect) {
        size_t bytesPerSample = m_wavBitsPerSample / 8;
        size_t frameBytes = bytesPerSample * channels;
        size_t bytesToRead = frames * frameBytes;

        // Ensure we don't read beyond data chunk
        std::streampos currentPos = m_wavFile.tellg();
        size_t offsetFromData = static_cast<size_t>(currentPos - m_wavDataStart);
        if (offsetFromData >= m_wavDataBytes) {
            m_eof = true;
            return 0;
        }

        if (offsetFromData + bytesToRead > m_wavDataBytes) {
            bytesToRead = m_wavDataBytes - offsetFromData;
            frames = bytesToRead / frameBytes;
        }

        std::vector<uint8_t> rawBytes(bytesToRead);
        m_wavFile.read(reinterpret_cast<char*>(rawBytes.data()), bytesToRead);
        size_t bytesRead = static_cast<size_t>(m_wavFile.gcount());
        size_t framesRead = bytesRead / frameBytes;

        if (m_wavAudioFormat == 3) { // IEEE Float 32
            std::memcpy(buffer, rawBytes.data(), framesRead * channels * sizeof(float));
        } else if (m_wavBitsPerSample == 16) {
            const int16_t* src16 = reinterpret_cast<const int16_t*>(rawBytes.data());
            constexpr float kInv = 1.0f / 32768.0f;
            for (size_t i = 0; i < framesRead * channels; ++i) {
                buffer[i] = static_cast<float>(src16[i]) * kInv;
            }
        } else if (m_wavBitsPerSample == 24) {
            constexpr float kInv = 1.0f / 8388608.0f;
            for (size_t i = 0; i < framesRead * channels; ++i) {
                size_t byteIdx = i * 3;
                int32_t val = (static_cast<int32_t>(rawBytes[byteIdx])) |
                              (static_cast<int32_t>(rawBytes[byteIdx + 1]) << 8) |
                              (static_cast<int32_t>(static_cast<int8_t>(rawBytes[byteIdx + 2])) << 16);
                buffer[i] = static_cast<float>(val) * kInv;
            }
        } else if (m_wavBitsPerSample == 32) {
            const int32_t* src32 = reinterpret_cast<const int32_t*>(rawBytes.data());
            constexpr double kInv = 1.0 / 2147483648.0;
            for (size_t i = 0; i < framesRead * channels; ++i) {
                buffer[i] = static_cast<float>(static_cast<double>(src32[i]) * kInv);
            }
        }

        m_currentFrame += framesRead;
        if (framesRead < frames || offsetFromData + bytesRead >= m_wavDataBytes) {
            m_eof = true;
        }
        return framesRead;
    }

    // FFmpeg pipe reading
    if (!m_pipeHandle) {
        m_eof = true;
        return 0;
    }

    size_t samplesToRead = frames * channels;
    size_t samplesRead = fread(buffer, sizeof(float), samplesToRead, m_pipeHandle);
    size_t framesRead = samplesRead / channels;

    m_currentFrame += framesRead;
    if (framesRead < frames) {
        m_eof = true;
    }
    return framesRead;
}

core::Result<void> FFmpegAudioDecoder::seek(double seconds) {
    if (!m_isOpen) {
        return core::Error::make(-1, "Decoder is not open", "FFmpegDecoder");
    }

    seconds = std::clamp(seconds, 0.0, m_durationSeconds);

    if (m_isWavDirect) {
        size_t targetFrame = static_cast<size_t>(seconds * static_cast<double>(m_format.sampleRate()));
        size_t bytesPerSample = m_wavBitsPerSample / 8;
        size_t byteOffset = targetFrame * (bytesPerSample * m_format.channels());

        if (byteOffset > m_wavDataBytes) byteOffset = m_wavDataBytes;

        m_wavFile.clear();
        m_wavFile.seekg(m_wavDataStart + static_cast<std::streamoff>(byteOffset));
        m_currentFrame = targetFrame;
        m_eof = (byteOffset >= m_wavDataBytes);
        return core::Result<void>::ok();
    }

    // For FFmpeg pipe, reopen process at requested timestamp
    if (openFfmpegPipe(m_filePath, seconds)) {
        m_eof = false;
        return core::Result<void>::ok();
    }

    return core::Error::make(-2, "Failed to seek in FFmpeg stream", "FFmpegDecoder");
}

double FFmpegAudioDecoder::positionSeconds() const noexcept {
    if (m_format.sampleRate() == 0) return 0.0;
    return static_cast<double>(m_currentFrame) / static_cast<double>(m_format.sampleRate());
}

} // namespace soniq::audio
