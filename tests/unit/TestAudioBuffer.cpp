#include "audio/AudioBuffer.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

void testAudioBuffer() {
    using namespace soniq::audio;

    std::cout << "\n=== Running AudioBuffer Unit Tests ===" << std::endl;

    AudioBuffer buf(1024, 2);
    assert(buf.frames() == 1024);
    assert(buf.channels() == 2);
    assert(buf.totalSamples() == 2048);

    // Int16 conversion test
    int16_t src16[4] = {0, 16384, -16384, 32767};
    buf.loadFromInt16(src16, 2, 2);
    assert(std::abs(buf.sample(0, 0) - 0.0f) < 1e-4f);
    assert(std::abs(buf.sample(0, 1) - 0.5f) < 1e-4f);
    assert(std::abs(buf.sample(1, 0) - (-0.5f)) < 1e-4f);
    assert(std::abs(buf.sample(1, 1) - 1.0f) < 1e-3f);

    // Peak level test
    assert(std::abs(buf.peakLevel() - 1.0f) < 1e-3f);
    assert(!buf.hasClipping(1.05f));

    // Gain scaling test
    buf.applyGain(0.5f);
    assert(std::abs(buf.peakLevel() - 0.5f) < 1e-3f);

    // Clipping detection test
    buf.applyGain(3.0f);
    assert(buf.hasClipping(1.0f));

    // Export to Int16 test
    int16_t dst16[4]{};
    buf.copyToInt16(dst16);
    // Samples should be clamped to [-32768, 32767]
    assert(dst16[3] == 32767);

    std::cout << "  [PASS] Sample format conversions, gain scaling, peak and clipping analysis" << std::endl;
    std::cout << "All AudioBuffer Unit Tests PASSED successfully.\n" << std::endl;
}
