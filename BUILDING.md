# SONIQ Build & Development Guide

## Build Requirements

- **C++ Compiler**: Modern C++20 compliant compiler:
  - Windows: Clang 16+ (LLVM-MinGW or Visual Studio Clang-CL) or GCC 13+ with UCRT
  - Linux: GCC 13+ or Clang 16+
  - macOS: Apple Clang 15+ / Xcode 15+
- **Build System**: CMake 3.22+ and Ninja
- **Qt Framework**: Qt 6.5 or later (Core, Gui, Qml, Quick, QuickControls2, Sql)
- **External Decoders**: FFmpeg binary installed on system PATH or at `C:\ffmpeg\ffmpeg.exe`

---

## Windows Build Instructions

### 1. Configure the Build
From the project root directory, run:
```powershell
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="D:/Qt/6.8.2/llvm-mingw_64" -DCMAKE_CXX_COMPILER=clang++
```

### 2. Compile Targets
```powershell
cmake --build build
```
This produces:
- `build/libsoniq_core.a`: The core audio engine, DSP, database, and services library
- `build/soniq_tests.exe`: The automated unit test suite
- `build/soniq_app.exe`: The complete desktop application

### 3. Run Automated Tests
```powershell
$llvmBin = "C:\Users\Rohan JM\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin"
$qtBin = "D:\Qt\6.8.2\llvm-mingw_64\bin"
$env:PATH = "$llvmBin;$qtBin;$env:PATH"

# Run with CTest
ctest --test-dir build --output-on-failure

# Or execute test binary directly
& "build\soniq_tests.exe"
```

### 4. Run SONIQ Desktop Application
- **Direct Root Execution**: Double-click `soniq_app.exe` directly in the project root directory. It runs seamlessly with zero external DLL requirements.
- **Standalone Portable Folder**: Double-click `dist/soniq_app.exe`. The `dist/` folder contains all self-contained Qt 6.8.2 libraries, MinGW UCRT runtime, and platform plugins, making it completely portable to any Windows 10/11 x64 machine.
- **Developer Execution from Build Folder**:
```powershell
$llvmBin = "C:\Users\Rohan JM\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin"
$qtBin = "D:\Qt\6.8.2\llvm-mingw_64\bin"
$env:PATH = "$llvmBin;$qtBin;$env:PATH"

& "build\soniq_app.exe"
```

---

## Linux Build Instructions
```bash
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="/usr/lib/qt6"
cmake --build build
ctest --test-dir build --output-on-failure
./build/soniq_app
```

---

## macOS Build Instructions
```bash
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
cmake --build build
ctest --test-dir build --output-on-failure
./build/soniq_app
```
