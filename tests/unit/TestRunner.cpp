#include <QCoreApplication>
#include <iostream>

// Declarations of individual test suites
void testAudioFormat();
void testAudioBuffer();
void runDspTests();
void testPlaybackQueue();
void testDatabase();
void testAudioEngine();
void runDsdTests();
void runArtworkAndGaplessTests();

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "==================================================" << std::endl;
    std::cout << "  SONIQ AUTOMATED TEST SUITE (PHASES 1, 2, 3)     " << std::endl;
    std::cout << "  Testing Audio Engine, DSP, DSD, DoP, Gapless, DB" << std::endl;
    std::cout << "==================================================" << std::endl;

    try {
        testAudioFormat();
        testAudioBuffer();
        runDspTests();
        testPlaybackQueue();
        testDatabase();
        testAudioEngine();
        runDsdTests();
        runArtworkAndGaplessTests();

        std::cout << "==================================================" << std::endl;
        std::cout << "  ✓ ALL SONIQ UNIT & INTEGRATION TESTS PASSED!    " << std::endl;
        std::cout << "==================================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "TEST FAILURE: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "TEST FAILURE: Unknown exception occurred." << std::endl;
        return 1;
    }
}
