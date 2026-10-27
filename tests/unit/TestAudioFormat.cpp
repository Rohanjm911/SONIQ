#include "core/AudioFormat.hpp"
#include <cassert>
#include <iostream>

void testAudioFormat() {
    using namespace soniq::core;

    std::cout << "\n=== Running AudioFormat Unit Tests ===" << std::endl;

    // CD Standard 44.1kHz / 16-bit Stereo
    AudioFormat cdFormat(44100, 2, 16, SampleFormat::Int16, ChannelLayout::Stereo, "PCM", "WAV");
    assert(cdFormat.isValid());
    assert(cdFormat.bytesPerSample() == 2);
    assert(cdFormat.frameSize() == 4);
    assert(cdFormat.bytesPerSecond() == 44100 * 4);
    assert(cdFormat.isLossless());
    assert(!cdFormat.isHiRes());

    // Hi-Res Studio Master 96kHz / 24-bit Stereo FLAC
    AudioFormat hiResFlac(96000, 2, 24, SampleFormat::Int24, ChannelLayout::Stereo, "FLAC", "FLAC");
    assert(hiResFlac.isValid());
    assert(hiResFlac.bytesPerSample() == 3);
    assert(hiResFlac.frameSize() == 6);
    assert(hiResFlac.isLossless());
    assert(hiResFlac.isHiRes());

    // MP3 44.1kHz / 16-bit 320kbps
    AudioFormat mp3Format(44100, 2, 16, SampleFormat::Float32, ChannelLayout::Stereo, "MP3", "MP3", 320);
    assert(mp3Format.isValid());
    assert(!mp3Format.isLossless());
    assert(!mp3Format.isHiRes());

    // Bit-perfect compatibility check
    AudioFormat identicalFlac(96000, 2, 24, SampleFormat::Int24, ChannelLayout::Stereo, "FLAC", "FLAC");
    assert(hiResFlac.isBitPerfectCompatibleWith(identicalFlac));
    assert(!hiResFlac.isBitPerfectCompatibleWith(cdFormat));

    // Time conversions
    uint64_t frames = cdFormat.secondsToFrames(2.5);
    assert(frames == 44100 * 2.5);
    double sec = cdFormat.framesToSeconds(frames);
    assert(std::abs(sec - 2.5) < 1e-5);

    std::cout << "  [PASS] Format metrics, lossless/hi-res identification, bit-perfect checks" << std::endl;
    std::cout << "All AudioFormat Unit Tests PASSED successfully.\n" << std::endl;
}
