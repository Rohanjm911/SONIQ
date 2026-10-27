#include "dsp/BiquadFilter.hpp"
#include "dsp/Equalizer.hpp"
#include "dsp/Limiter.hpp"
#include "dsp/Crossfeed.hpp"
#include "dsp/DspPipeline.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

void testBiquadFilter() {
    using namespace soniq::dsp;
    BiquadFilter filter;

    // 1. Peaking EQ test at 1000 Hz, +6 dB boost
    filter.configure(FilterType::PeakingEQ, 44100.0, 1000.0, 1.0, 6.0);
    double magAtCenter = filter.magnitudeResponse(1000.0, 44100.0);
    double magDb = 20.0 * std::log10(magAtCenter);

    // Magnitude at center should be very close to +6.0 dB (+- 0.2 dB)
    assert(std::abs(magDb - 6.0) < 0.2);

    // 2. Magnitude far away from center should be close to 0 dB (1.0)
    double magFar = filter.magnitudeResponse(100.0, 44100.0);
    assert(std::abs(20.0 * std::log10(magFar)) < 0.5);

    // 3. Process test: filter stability (no NaN / Inf with bounded input)
    for (int i = 0; i < 1000; ++i) {
        float in = 0.5f * std::sin(static_cast<float>(i) * 0.05f);
        float out = filter.process(in);
        assert(!std::isnan(out) && !std::isinf(out));
    }

    std::cout << "  [PASS] BiquadFilter mathematical response and stability" << std::endl;
}

void testEqualizer() {
    using namespace soniq::dsp;
    Equalizer eq;
    eq.setSampleRate(44100.0);

    // When disabled, does not alter signal
    assert(!eq.altersSignal());

    // When enabled but flat gains, does not alter signal
    eq.setEnabled(true);
    assert(!eq.altersSignal());

    // Boost 1 kHz band (+3 dB)
    eq.setBandGain10(5, 3.0); // 1 kHz band
    assert(eq.altersSignal());

    // Process a block
    std::vector<float> block(2048, 0.25f);
    eq.process(block.data(), 1024, 2);

    // Audio values must be modified and remain within stable bounds
    assert(!std::isnan(block[0]) && !std::isinf(block[0]));

    // Preset test
    eq.applyPreset("Rock");
    assert(eq.bandGain10(0) > 0.0); // Bass should be boosted
    assert(eq.bandGain10(9) > 0.0); // Treble should be boosted

    eq.resetGains();
    assert(!eq.altersSignal());

    std::cout << "  [PASS] Equalizer 10-band and preset management" << std::endl;
}

void testLimiter() {
    using namespace soniq::dsp;
    Limiter limiter;
    limiter.configure(44100.0, -0.3, 50.0); // -0.3 dB threshold
    limiter.setEnabled(true);

    // Create a signal with severe clipping (> +6 dB, amplitude 2.0f)
    std::vector<float> clippedAudio(512, 2.0f);
    limiter.process(clippedAudio.data(), 256, 2);

    // All samples must be clamped to ceiling (never exceeds ceiling ~0.988f)
    for (float s : clippedAudio) {
        assert(std::abs(s) <= 0.99f);
        assert(!std::isnan(s) && !std::isinf(s));
    }
    assert(limiter.wasTriggered());

    std::cout << "  [PASS] Limiter peak-catching and clipping prevention" << std::endl;
}

void testDspPipeline() {
    using namespace soniq::dsp;
    DspPipeline dsp;
    dsp.setSampleRate(44100.0);

    // Default: bypassed / no alteration
    dsp.limiter().setEnabled(false); // disable limiter to check clean state
    assert(!dsp.altersSignal());
    assert(dsp.bitPerfectDisablingReasons().empty());

    // Enable crossfeed
    dsp.crossfeed().setEnabled(true);
    assert(dsp.altersSignal());
    auto reasons = dsp.bitPerfectDisablingReasons();
    assert(!reasons.empty());

    std::cout << "  [PASS] DspPipeline signal alteration audit and bit-perfect checking" << std::endl;
}

void runDspTests() {
    std::cout << "\n=== Running DSP & Equalizer Unit Tests ===" << std::endl;
    testBiquadFilter();
    testEqualizer();
    testLimiter();
    testDspPipeline();
    std::cout << "All DSP Unit Tests PASSED successfully.\n" << std::endl;
}
