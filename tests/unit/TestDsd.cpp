#include "audio/dsd/DsdProcessor.hpp"
#include "audio/dsd/DsdAudioDecoder.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <filesystem>

using namespace soniq::audio::dsd;

void testDopPackingAndUnpacking() {
    std::cout << "Testing DoP 16-bit framing & alternating marker verification..." << std::endl;

    constexpr size_t kFrames = 64;
    std::vector<uint16_t> ch0(kFrames);
    std::vector<uint16_t> ch1(kFrames);

    // Fill with distinct test bit patterns
    for (size_t i = 0; i < kFrames; ++i) {
        ch0[i] = static_cast<uint16_t>(0x1234 + i * 7);
        ch1[i] = static_cast<uint16_t>(0xABCD - i * 3);
    }

    std::vector<int32_t> pcm24(kFrames * 2, 0);
    uint8_t markerState = 0;

    DsdProcessor::packDoP(ch0.data(), ch1.data(), pcm24.data(), kFrames, markerState);

    // 1. Verify markers alternate between 0x05 and 0xFA
    for (size_t f = 0; f < kFrames; ++f) {
        uint8_t markerL = static_cast<uint8_t>((pcm24[f * 2 + 0] >> 16) & 0xFF);
        uint8_t markerR = static_cast<uint8_t>((pcm24[f * 2 + 1] >> 16) & 0xFF);

        uint8_t expected = (f % 2 == 0) ? DsdProcessor::kDopMarker0 : DsdProcessor::kDopMarker1;
        assert(markerL == expected && "Left channel marker mismatch in DoP frame");
        assert(markerR == expected && "Right channel marker mismatch in DoP frame");
    }

    // 2. Verify stream detection
    assert(DsdProcessor::isDopStream(pcm24.data(), kFrames) && "DoP stream detector failed to identify valid DoP frames");

    // 3. Unpack and verify 100% bit preservation
    std::vector<uint16_t> unpacked0(kFrames, 0);
    std::vector<uint16_t> unpacked1(kFrames, 0);

    bool ok = DsdProcessor::unpackDoP(pcm24.data(), unpacked0.data(), unpacked1.data(), kFrames);
    assert(ok && "DoP unpack failed");

    for (size_t i = 0; i < kFrames; ++i) {
        assert(unpacked0[i] == ch0[i] && "Unpacked channel 0 bits differ from original DSD");
        assert(unpacked1[i] == ch1[i] && "Unpacked channel 1 bits differ from original DSD");
    }

    std::cout << "  -> DoP integer packing, alternating markers, and bit-exact extraction PASSED." << std::endl;
}

void testDopFloatPackingAndUnpacking() {
    std::cout << "Testing DoP float container packing & detection..." << std::endl;

    constexpr size_t kFrames = 32;
    std::vector<uint16_t> ch0(kFrames);
    std::vector<uint16_t> ch1(kFrames);

    for (size_t i = 0; i < kFrames; ++i) {
        ch0[i] = static_cast<uint16_t>(0xA5A5 ^ i);
        ch1[i] = static_cast<uint16_t>(0x5A5A ^ i);
    }

    std::vector<float> pcmFloat(kFrames * 2, 0.0f);
    uint8_t markerState = 0;

    DsdProcessor::packDoPFloat(ch0.data(), ch1.data(), pcmFloat.data(), kFrames, markerState);

    // Verify float DoP detection
    assert(DsdProcessor::isDopStreamFloat(pcmFloat.data(), kFrames) && "Float DoP detector failed");

    // Unpack and verify
    std::vector<uint16_t> unpacked0(kFrames, 0);
    std::vector<uint16_t> unpacked1(kFrames, 0);

    bool ok = DsdProcessor::unpackDoPFloat(pcmFloat.data(), unpacked0.data(), unpacked1.data(), kFrames);
    assert(ok && "Float DoP unpack failed");

    for (size_t i = 0; i < kFrames; ++i) {
        assert(unpacked0[i] == ch0[i] && "Float DoP unpacked ch0 bit mismatch");
        assert(unpacked1[i] == ch1[i] && "Float DoP unpacked ch1 bit mismatch");
    }

    std::cout << "  -> DoP float packing and unpacking PASSED." << std::endl;
}

void testDsdDecimation() {
    std::cout << "Testing 1-bit DSD-to-PCM decimation filter (40 kHz cutoff FIR)..." << std::endl;

    // DSD silence is alternating 01010101 (0x55 or 0x69)
    constexpr size_t kBytes = 256;
    std::vector<uint8_t> silence(kBytes, 0x69);

    std::vector<float> outPcm(kBytes * 8 / 32, 0.0f);
    size_t outSamples = 0;

    DsdProcessor::decimateDsdToPcm(silence.data(), kBytes, true, outPcm.data(), outSamples, 32);

    assert(outSamples == 64 && "Decimation sample count mismatch for 32:1 factor");

    // Check that silence produces near zero output (DC balanced)
    for (size_t i = 16; i < outSamples - 16; ++i) { // avoid filter startup transient
        assert(std::abs(outPcm[i]) < 0.25f && "DSD silence produced excessive DC offset");
        assert(!std::isnan(outPcm[i]) && !std::isinf(outPcm[i]) && "NaN/Inf in decimated PCM");
    }

    std::cout << "  -> DSD-to-PCM FIR decimation filter PASSED." << std::endl;
}

void testSyntheticDsfParsing() {
    std::cout << "Testing DSF binary header parser with synthetic stream..." << std::endl;

    namespace fs = std::filesystem;
    std::string tempDsf = "test_synthetic.dsf";

    // Write a minimal valid DSF file
    std::ofstream out(tempDsf, std::ios::binary);
    assert(out.is_open());

    // 1. 'DSD ' chunk (28 bytes)
    out.write("DSD ", 4);
    uint64_t dsdSize = 28;
    uint64_t fileSize = 28 + 52 + 12 + 4096 * 2;
    uint64_t metaOffset = 0;
    out.write(reinterpret_cast<char*>(&dsdSize), 8);
    out.write(reinterpret_cast<char*>(&fileSize), 8);
    out.write(reinterpret_cast<char*>(&metaOffset), 8);

    // 2. 'fmt ' chunk (52 bytes)
    out.write("fmt ", 4);
    uint64_t fmtSize = 52;
    uint32_t fmtVersion = 1;
    uint32_t fmtId = 0;
    uint32_t chType = 2; // stereo
    uint32_t chNum = 2;
    uint32_t sampleRate = 2822400; // DSD64
    uint32_t bitsPerSample = 1;
    uint64_t sampleCount = 2822400; // 1 second
    uint32_t blockSize = 4096;
    uint32_t reserved = 0;

    out.write(reinterpret_cast<char*>(&fmtSize), 8);
    out.write(reinterpret_cast<char*>(&fmtVersion), 4);
    out.write(reinterpret_cast<char*>(&fmtId), 4);
    out.write(reinterpret_cast<char*>(&chType), 4);
    out.write(reinterpret_cast<char*>(&chNum), 4);
    out.write(reinterpret_cast<char*>(&sampleRate), 4);
    out.write(reinterpret_cast<char*>(&bitsPerSample), 4);
    out.write(reinterpret_cast<char*>(&sampleCount), 8);
    out.write(reinterpret_cast<char*>(&blockSize), 4);
    out.write(reinterpret_cast<char*>(&reserved), 4);

    // 3. 'data' chunk
    out.write("data", 4);
    uint64_t dataChunkSize = 12 + 4096 * 2;
    out.write(reinterpret_cast<char*>(&dataChunkSize), 8);

    std::vector<char> block(4096 * 2, 0x69);
    out.write(block.data(), block.size());
    out.close();

    DsdFileInfo info;
    bool parsed = DsdParser::parseFile(tempDsf, info);
    assert(parsed && "Failed to parse synthetic DSF file");
    assert(info.type == DsdFormatType::DSF && "DSF format type mismatch");
    assert(info.sampleRate == 2822400 && "DSF sample rate mismatch");
    assert(info.channels == 2 && "DSF channel count mismatch");
    assert(info.blockSize == 4096 && "DSF block size mismatch");
    assert(std::abs(info.durationSeconds - 1.0) < 1e-4 && "DSF duration mismatch");

    fs::remove(tempDsf);
    std::cout << "  -> Synthetic DSF parser verification PASSED." << std::endl;
}

void runDsdTests() {
    std::cout << "========================================" << std::endl;
    std::cout << "    RUNNING DSD & DoP TEST SUITE        " << std::endl;
    std::cout << "========================================" << std::endl;

    testDopPackingAndUnpacking();
    testDopFloatPackingAndUnpacking();
    testDsdDecimation();
    testSyntheticDsfParsing();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL DSD & DoP TESTS PASSED (4/4)      " << std::endl;
    std::cout << "========================================" << std::endl;
}
