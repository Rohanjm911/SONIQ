# SONIQ Troubleshooting Guide

## 1. Runtime Errors & DLL Resolution (Windows)

### Issue: `STATUS_ENTRYPOINT_NOT_FOUND (0xc0000139)` on startup
- **Cause**: Windows dynamically linked a different compiler runtime DLL (`libc++.dll` or `libwinpthread-1.dll`) from another compiler distribution earlier in your `PATH`.
- **Solution**: Prepend your Clang/LLVM toolchain directory and Qt binary directory to `PATH` before launching:
  ```powershell
  $llvmBin = "C:\Users\Rohan JM\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin"
  $qtBin = "D:\Qt\6.8.2\llvm-mingw_64\bin"
  $env:PATH = "$llvmBin;$qtBin;$env:PATH"
  ```

---

## 2. Audio Output Issues

### Issue: Bit-Perfect playback status displays "Unavailable"
- **Reason 1 - Software Volume Attenuation**: Software volume is set below 1.0 (100%). To preserve original raw bitstream values, set the volume slider to 100% and use your DAC or amplifier hardware volume knob.
- **Reason 2 - Active DSP**: Equalizer, Crossfeed, Stereo Width, or Limiter is active. In the Equalizer screen, toggle the master switch to "BYPASS" to allow unadulterated bitstream passthrough.
- **Reason 3 - Shared Output Mode**: In Shared Mode, the Windows Audio Engine mixer resamples and mixes stream samples. In Settings -> Audio Output, toggle "WASAPI Exclusive Mode" to lock direct access to the audio endpoint.

### Issue: "Exclusive mode rejected by Windows audio driver"
- **Cause**: Another desktop application (such as a DAW, another media player, or web browser) has acquired exclusive ownership of the target audio device, or the soundcard driver does not support exclusive mode at the requested sampling rate.
- **Behavior**: SONIQ automatically and safely falls back to Shared Mode without crashing, notifying the user in the Diagnostics/Audio Inspector interface.

---

## 3. Library & Decoding Issues

### Issue: Track metadata or artwork is missing
- **Cause**: The audio file container might lack standard ID3v2/Vorbis/FLAC metadata blocks.
- **Behavior**: SONIQ falls back to directory structure and file stem name parsing (e.g., `Artist - Title.flac`), preserving playback capability.

### Issue: FFmpeg pipe spawn failure
- **Cause**: `ffmpeg.exe` is not found on PATH or in `C:\ffmpeg`.
- **Solution**: Ensure FFmpeg is installed and accessible via PATH, or download static FFmpeg binaries to `C:\ffmpeg`. Uncompressed WAV and PCM files continue to play directly via the built-in direct PCM engine.
