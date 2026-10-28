<div align="center">
  <img src="ui/assets/logo_full.png" alt="SONIQ Logo" width="420" />

  <br />

  <a href="https://git.io/typing-svg">
    <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=16&duration=2500&pause=1000&color=FA2D48&center=true&vCenter=true&width=520&lines=Pure+Sound.+Infinite+Possibilities.;Bit-Perfect+Hardware+WASAPI+Playback;Native+DSD+%2F+DoP+Streaming+%26+FIR+Decimation;Studio+DSP+Pipeline+%26+10-Band+Equalizer;Zero+Telemetry.+Offline-First.+Open+Source." alt="SONIQ Typing Tagline" />
  </a>

  <br />

  <img src="ui/assets/soundwave_animated.svg" alt="Animated Audiophile Soundwave" width="100%" />

  <br />

  [![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/20)
  [![Qt 6](https://img.shields.io/badge/Qt-6.8-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
  [![WASAPI](https://img.shields.io/badge/Audio-WASAPI%20Bit--Perfect-0078D4?logo=windows&logoColor=white)]()
  [![DSD / DoP](https://img.shields.io/badge/Hi--Res-Native%20DSD%20%2F%20DoP-FF6F00)]()
  [![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
</div>

---

SONIQ is a modern, offline-first, cross-platform audiophile desktop music player for Windows, Linux, and macOS. Built with modern **C++20**, **Qt 6**, **Qt Quick/QML**, and native platform audio drivers (**WASAPI Exclusive Mode** on Windows), SONIQ delivers bit-perfect audio reproduction, studio-grade 64-bit DSP filtering, high-performance local library indexing, and a clean, responsive desktop user experience.

---

## Key Highlights

- **Pure Audiophile Audio Path**: Bit-perfect playback verification with zero unneeded software volume scaling, resampling, or dithering in exclusive output mode.
- **Dedicated Audio Threading**: Audio engine, decoding, and signal pipeline run decoupled from the UI thread, ensuring zero stuttering during UI animations, scanning, or window resizing.
- **Native DSD & DoP Engine**: Native Direct Stream Digital bitstream handling for `.dsf` and `.dff` files via DoP v1.1 (DSD over PCM at 176.4/352.8 kHz) for bit-perfect hardware playback and 64-tap linear-phase FIR decimation (40 kHz cutoff) for software rendering.
- **Gapless Transitions & Lookahead Buffering**: Seamless zero-gap track transitions with background lookahead pre-buffering, zero-restart endpoint reuse, and optional equal-power constant-acoustic-power crossfading.
- **Universal Audio Decoding**: Direct low-latency PCM RIFF/WAV decoding, DSD/DSF/DFF engine, and universal FFmpeg audio streaming (FLAC, ALAC, WAV, AIFF, MP3, AAC, OGG Vorbis, Opus, WavPack, APE, and more).
- **Format-Aware Artwork Extraction**: High-speed extraction of embedded album artwork from FLAC picture blocks, MP3 ID3v2 APIC tags, and stream copy to persistent disk cache.
- **Studio DSP Engine**:
  - 10-Band Graphic EQ (ISO Standard Octave bands)
  - 31-Band Graphic EQ (1/3 Octave bands)
  - Parametric EQ (Peaking, Low Shelf, High Shelf, High Pass, Low Pass, Notch)
  - Master Preamp Gain (-24 dB to +24 dB)
  - Bauer Stereophonic-to-Binaural Crossfeed (headphone acoustic fatigue reduction)
  - Lookahead Peak Limiter & Transparent Anti-Clipping
  - Bit-Perfect Integrity Audit (identifies and explains any active DSP modifying raw samples)
- **Offline-First & Privacy Conscious**:
  - Zero mandatory accounts or online telemetry
  - Local SQLite database with versioned schema migrations (v1 and v2)
  - Asynchronous recursive library scanner
  - Full playlist management and instant search
- **Design System & Presentation**:
  - Unified design tokens for dark and light themes
  - Persistent mini-player with scrubbing timeline, album artwork, and real-time spectrum visualizer
  - Centerpiece Now Playing screen and slide-in Playing Queue drawer
  - Technical Audio File Inspector showing container, codec, sample rate, bit depth, channel configuration, and bit-perfect eligibility audit

---

## Technology Stack

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++20" />
  &nbsp;
  <img src="https://img.shields.io/badge/Qt_6.8-41CD52?style=for-the-badge&logo=qt&logoColor=white" alt="Qt 6.8" />
  &nbsp;
  <img src="https://img.shields.io/badge/Windows_WASAPI-0078D4?style=for-the-badge&logo=windows&logoColor=white" alt="Windows WASAPI" />
  &nbsp;
  <img src="https://img.shields.io/badge/FFmpeg-007808?style=for-the-badge&logo=ffmpeg&logoColor=white" alt="FFmpeg" />
  &nbsp;
  <img src="https://img.shields.io/badge/SQLite_3-003B57?style=for-the-badge&logo=sqlite&logoColor=white" alt="SQLite" />
  &nbsp;
  <img src="https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
  &nbsp;
  <img src="https://img.shields.io/badge/Ninja-000000?style=for-the-badge&logo=ninja&logoColor=white" alt="Ninja" />
  &nbsp;
  <img src="https://img.shields.io/badge/LLVM_Clang-900000?style=for-the-badge&logo=llvm&logoColor=white" alt="LLVM/Clang" />
</p>

<br />

| Layer | Technology | Details |
| :--- | :--- | :--- |
| **Language** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" width="20" height="20" align="absmiddle" alt="C++" /> **C++20** | Modern ISO C++20 standard (`std::span`, `std::format`, `std::atomic`, RAII, smart pointers) |
| **UI Framework** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/qt/qt-original.svg" width="20" height="20" align="absmiddle" alt="Qt 6" /> **Qt 6.8+ Quick / QML** | Declarative QML UI, Hardware-Accelerated SceneGraph, reactive signal-slot bindings |
| **Audio Engine** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="20" height="20" align="absmiddle" alt="WASAPI" /> **Windows Core Audio (WASAPI)** | Event-driven Exclusive Mode (bit-perfect direct hardware) & Shared Mode with autoconvert fallback |
| **Audio Decoding** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/ffmpeg/ffmpeg-original.svg" width="20" height="20" align="absmiddle" alt="FFmpeg" /> **FFmpeg & Direct WAV Parser** | Universal 32-bit float streaming pipeline (FLAC, ALAC, DSD/DoP, MP3, AAC, OGG) + direct RIFF header reader |
| **Database** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/sqlite/sqlite-original.svg" width="20" height="20" align="absmiddle" alt="SQLite" /> **SQLite 3 (Qt6Sql)** | High-concurrency WAL mode journal, prepared statements, automated schema migrations (v1/v2) |
| **Build System** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cmake/cmake-original.svg" width="20" height="20" align="absmiddle" alt="CMake" /> **CMake & Ninja** | Fast parallel build generation, automated MOC/RCC compilation, modular architecture |
| **Compiler Toolchain** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/llvm/llvm-original.svg" width="20" height="20" align="absmiddle" alt="LLVM" /> **LLVM / Clang (MinGW-w64)** | High-performance C++20 compiler with LLD linker and full UCRT runtime integration |
| **Testing Suite** | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-plain.svg" width="20" height="20" align="absmiddle" alt="CTest" /> **CTest & Automated Suite** | Unit and integration test suite covering DSP filters, DSD DoP packing, database, and gapless audio |

---

## Directory Structure

```
SONIQ/
├── CMakeLists.txt          # Root CMake build definition
├── resources.qrc           # Qt QML resource collection
├── app/
│   └── main.cpp            # Main application bootstrap and service wiring
├── src/
│   ├── core/               # Types, AudioFormat, Logger, Result, Error
│   ├── audio/              # PlaybackEngine, Decoders, Buffer, Queue
│   │   └── output/         # IAudioOutput, WASAPIAudioOutput, NullAudioOutput
│   ├── dsp/                # BiquadFilter, Equalizer, Limiter, Crossfeed, DspPipeline
│   ├── database/           # SQLite DatabaseManager and Schema Migrations
│   ├── library/            # LibraryScanner and Track models
│   ├── metadata/           # AudioProbe (container & codec inspection)
│   └── services/           # PlaybackController, LibraryModel, DspService, SettingsService
├── ui/
│   ├── main.qml            # Application root window and layout shell
│   ├── themes/             # Design tokens: Colors, Typography, Motion, Theme
│   ├── components/         # Reusable SoniqButton, SoniqSlider, SoniqTrackRow, SoniqSpectrum
│   ├── panels/             # NavigationSidebar, MiniPlayer, QueueDrawer
│   └── screens/            # HomeScreen, SongsScreen, NowPlayingScreen, EqualizerScreen, SettingsScreen
├── tests/
│   └── unit/               # Automated unit tests for DSP, Audio Engine, Database, Queue
└── docs/                   # Architectural and operational documentation
```

---

## Quick Start: Building & Running

### Prerequisites
- CMake 3.22 or higher
- C++20 compliant compiler (Clang 16+, GCC 13+, or MSVC 2022)
- Qt 6.5+ (Qt Core, Gui, Qml, Quick, QuickControls2, Sql)
- Ninja build tool

### Commands (Windows PowerShell)

```powershell
# 1. Configure the build with Ninja and your Qt 6 installation
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="D:/Qt/6.8.2/llvm-mingw_64" -DCMAKE_CXX_COMPILER=clang++

# 2. Build the core library, desktop application, and test suite
cmake --build build

# 3. Run the automated test suite via CTest
$env:PATH = "C:\Users\Rohan JM\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin;D:\Qt\6.8.2\llvm-mingw_64\bin;$env:PATH"
ctest --test-dir build --output-on-failure

# 4. Launch the SONIQ Desktop Application
# Direct root execution (zero DLL dependencies):
.\soniq_app.exe

# Or run the portable standalone bundle:
.\dist\soniq_app.exe
```
