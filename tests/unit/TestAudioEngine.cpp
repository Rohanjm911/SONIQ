#include "audio/AudioPlaybackEngine.hpp"
#include "audio/FFmpegAudioDecoder.hpp"
#include "audio/output/NullAudioOutput.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

void testAudioEngine() {
    using namespace soniq::audio;
    using namespace soniq::core;

    std::cout << "\n=== Running AudioPlaybackEngine Unit Tests ===" << std::endl;

    // 1. Test NullAudioOutput
    auto nullOut = std::make_unique<NullAudioOutput>();
    AudioFormat fmt(96000, 2, 24, SampleFormat::Float32, ChannelLayout::Stereo, "PCM", "WAV");

    auto openRes = nullOut->open(fmt, "", OutputMode::Exclusive);
    assert(openRes.isOk());

    nullOut->start();
    assert(nullOut->isRunning());

    // In exclusive mode at 100% volume, bit-perfect status must be Verified
    nullOut->setVolume(1.0f);
    assert(nullOut->bitPerfectStatus() == BitPerfectStatus::Verified);

    // Attenuating volume must disable bit-perfect status
    nullOut->setVolume(0.8f);
    assert(nullOut->bitPerfectStatus() == BitPerfectStatus::Unavailable);
    assert(nullOut->bitPerfectStatusExplanation().find("attenuated") != std::string::npos);

    nullOut->setVolume(1.0f);
    assert(nullOut->bitPerfectStatus() == BitPerfectStatus::Verified);

    // Write samples
    std::vector<float> testSamples(2048, 0.75f);
    size_t written = nullOut->write(testSamples.data(), 1024);
    assert(written == 1024);
    assert(nullOut->totalFramesWritten() == 1024);
    assert(nullOut->capturedSamples().size() == 2048);
    assert(std::abs(nullOut->capturedSamples()[0] - 0.75f) < 1e-4f);

    nullOut->stop();
    assert(!nullOut->isRunning());

    // 2. Test AudioPlaybackEngine State Machine
    AudioPlaybackEngine engine;
    assert(engine.state() == PlaybackState::Stopped);

    engine.setVolume(0.9f);
    assert(std::abs(engine.volume() - 0.9f) < 1e-4f);

    engine.setMuted(true);
    assert(engine.isMuted());
    engine.setMuted(false);
    assert(!engine.isMuted());

    // 3. Test FFmpegAudioDecoder on file with spaces
    {
        namespace fs = std::filesystem;
        fs::path tempDir = fs::temp_directory_path() / "soniq_decoder_test with spaces";
        fs::create_directories(tempDir);
        fs::path wavFile = tempDir / "test space song.wav";

        // Create 0.25 second valid 44.1kHz stereo PCM WAV file
        std::ofstream out(wavFile, std::ios::binary);
        const uint32_t sampleRate = 44100;
        const uint16_t channels = 2;
        const uint16_t bitsPerSample = 16;
        const uint32_t numSamples = sampleRate / 4;
        const uint32_t dataBytes = numSamples * channels * (bitsPerSample / 8);
        const uint32_t riffBytes = 36 + dataBytes;
        const uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
        const uint16_t blockAlign = channels * (bitsPerSample / 8);

        out.write("RIFF", 4);
        out.write(reinterpret_cast<const char*>(&riffBytes), 4);
        out.write("WAVE", 4);
        out.write("fmt ", 4);
        uint32_t fmtSize = 16;
        out.write(reinterpret_cast<const char*>(&fmtSize), 4);
        uint16_t audioFormat = 1;
        out.write(reinterpret_cast<const char*>(&audioFormat), 2);
        out.write(reinterpret_cast<const char*>(&channels), 2);
        out.write(reinterpret_cast<const char*>(&sampleRate), 4);
        out.write(reinterpret_cast<const char*>(&byteRate), 4);
        out.write(reinterpret_cast<const char*>(&blockAlign), 2);
        out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
        out.write("data", 4);
        out.write(reinterpret_cast<const char*>(&dataBytes), 4);
        std::vector<int16_t> pcmData(numSamples * channels, 1000);
        out.write(reinterpret_cast<const char*>(pcmData.data()), dataBytes);
        out.close();

        FFmpegAudioDecoder decoder;
        auto res = decoder.open(wavFile.string());
        assert(res.isOk() && "Decoder must open audio file with spaces");
        assert(decoder.isOpen());
        assert(decoder.format().sampleRate() == 44100);

        std::vector<float> readBuf(512 * 2);
        size_t framesRead = decoder.readFrames(readBuf.data(), 512);
        assert(framesRead > 0 && "Decoder must read valid audio frames");
        decoder.close();

        std::error_code ec;
        fs::remove_all(tempDir, ec);
    }

    std::cout << "  [PASS] Output sink verification, bit-perfect validation, engine state machine, and FFmpeg decoder file reading" << std::endl;
    std::cout << "All AudioPlaybackEngine Unit Tests PASSED successfully.\n" << std::endl;
}
