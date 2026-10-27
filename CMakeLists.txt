cmake_minimum_required(VERSION 3.22)

project(SONIQ
    VERSION 1.0.0
    DESCRIPTION "Pure Sound. Infinite Possibilities. Audiophile Desktop Music Player"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automatic MOC and resource compilation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 modules
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Sql
)

# Core library sources
set(SONIQ_CORE_SOURCES
    src/core/Logger.cpp
    src/core/AudioFormat.cpp
    src/audio/AudioBuffer.cpp
    src/audio/AudioPlaybackEngine.cpp
    src/audio/FFmpegAudioDecoder.cpp
    src/audio/PlaybackQueue.cpp
    src/audio/output/NullAudioOutput.cpp
    src/dsp/BiquadFilter.cpp
    src/dsp/Equalizer.cpp
    src/dsp/Limiter.cpp
    src/dsp/Crossfeed.cpp
    src/dsp/DspPipeline.cpp
    src/metadata/AudioProbe.cpp
    src/metadata/ArtworkExtractor.cpp
    src/audio/dsd/DsdProcessor.cpp
    src/audio/dsd/DsdAudioDecoder.cpp
    src/database/DatabaseManager.cpp
    src/library/LibraryScanner.cpp
    src/services/PlaybackController.cpp
    src/services/LibraryModel.cpp
    src/services/AudioInspectorService.cpp
    src/services/DspService.cpp
    src/services/SettingsService.cpp
)

if(WIN32)
    list(APPEND SONIQ_CORE_SOURCES src/audio/output/WASAPIAudioOutput.cpp)
endif()

add_library(soniq_core STATIC ${SONIQ_CORE_SOURCES})

target_include_directories(soniq_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(soniq_core PUBLIC
    Qt6::Core
    Qt6::Sql
    Qt6::Gui
    Qt6::Quick
)

if(WIN32)
    target_link_libraries(soniq_core PUBLIC
        ole32
        avrt
        ksuser
    )
endif()

# Main Application Executable
add_executable(soniq_app
    app/main.cpp
    resources.qrc
)

target_link_libraries(soniq_app PRIVATE
    soniq_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
)

# Automated Test Suite
enable_testing()

add_executable(soniq_tests
    tests/unit/TestRunner.cpp
    tests/unit/TestAudioFormat.cpp
    tests/unit/TestAudioBuffer.cpp
    tests/unit/TestDsp.cpp
    tests/unit/TestPlaybackQueue.cpp
    tests/unit/TestDatabase.cpp
    tests/unit/TestAudioEngine.cpp
    tests/unit/TestDsd.cpp
    tests/unit/TestArtworkAndGapless.cpp
)

target_link_libraries(soniq_tests PRIVATE
    soniq_core
    Qt6::Core
    Qt6::Sql
)

add_test(NAME SoniqAutomatedTests COMMAND soniq_tests)
