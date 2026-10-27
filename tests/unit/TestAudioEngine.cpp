#include "audio/AudioPlaybackEngine.hpp"
#include "audio/output/NullAudioOutput.hpp"
#include <cassert>
#include <iostream>

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

    std::cout << "  [PASS] Output sink verification, bit-perfect validation, and engine state machine" << std::endl;
    std::cout << "All AudioPlaybackEngine Unit Tests PASSED successfully.\n" << std::endl;
}
