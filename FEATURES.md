# SONIQ — Comprehensive Feature Specification

> **Pure Sound. Infinite Possibilities.**  
> An offline-first, bit-perfect, cross-platform audiophile music player built with **C++20** and **Qt 6.8.2**.

---

## 1. Audiophile Audio Playback Engine

* **Bit-Perfect Hardware Output (WASAPI Exclusive Mode):**
  * Direct kernel-level audio streaming that bypasses the Windows Audio Engine, `CAudioLimiter`, and Windows system mixer (`kmixer`).
  * Streams bit-exact, unaltered audio samples directly to your external USB DAC, sound card, or audio interface.
  * Automatic graceful fallback to WASAPI Shared Mode if another application or device holds exclusive access.

* **Native DSD & DoP Bitstream Engine:**
  * **Direct Stream Digital (DSD):** Direct playback support for `.dsf` and `.dff` (Super Audio CD) bitstream tracks.
  * **DoP v1.1 Unpacker (DSD over PCM):** Formats 1-bit DSD bitstreams into standard 16-bit PCM frames with alternating `0x05` and `0xFA` synchronization markers at 176.4 kHz (DSD64) or 352.8 kHz (DSD128) for hardware DoP-compatible DACs.
  * **Linear-Phase Decimation Filter:** High-order 64-tap linear-phase FIR decimation filter with a 40 kHz cutoff frequency for software conversion of 1-bit DSD to PCM on non-DSD sound cards.

* **Gapless Transitions & Lookahead Pre-Buffering:**
  * Multi-threaded lookahead pre-decoder pre-buffers the upcoming track into ring buffers before the active track finishes.
  * Seamless zero-latency boundary handovers without output device re-open clicks or stutter.
  * Configurable constant-acoustic-power (equal-power) crossfade transitions from 0 to 10 seconds.

* **Master-Quality Sampling Support:**
  * Handles audio streams from standard Red Book CD quality (44.1 kHz / 16-bit) up to **768 kHz / 32-bit floating point**.

---

## 2. Universal Format Compatibility

SONIQ supports virtually all uncompressed, lossless, DSD, and lossy audio containers:

| Category | Formats & Extensions | Bit Depth & Sample Rates |
|---|---|---|
| **Uncompressed PCM** | `.wav`, `.aiff`, `.aif` | 8-bit to 32-bit integer & float, up to 768 kHz |
| **Lossless Compression** | `.flac`, `.alac`, `.m4a`, `.wv` (WavPack), `.ape` (Monkey's Audio), `.tta`, `.tak` | Up to 24-bit / 192 kHz and 32-bit float |
| **Direct Stream Digital** | `.dsf`, `.dff` (DSD64, DSD128, DSD256) | 2.8224 MHz to 11.2896 MHz 1-bit |
| **Lossy Codecs** | `.mp3`, `.aac`, `.ogg` (Vorbis), `.opus`, `.wma`, `.ac3`, `.dts` | 32 kbps to 320 kbps, CBR/VBR, up to 48 kHz |

---

## 3. High-Performance Multi-Threaded Library Scanner

* **Asynchronous Background Indexing:**
  * Multi-threaded background scanner runs on an independent worker thread without dropping GUI frame rates.
  * Handles large multi-gigabyte collections and deeply nested directory trees.
* **Full Unicode & Path Normalization:**
  * Comprehensive Unicode, percent-encoding (`%20`), and Windows path spaces handling.
  * Seamlessly resolves drives, folders, and tracks with non-ASCII or international characters.
* **Live Scanning Progress Banner:**
  * Real-time progress notifications in the top UI banner displaying `(X / Y)` scanned file counts and current indexing state.
* **Thread-Safe SQLite Database with WAL Mode:**
  * Embedded SQLite database running in **WAL (Write-Ahead Logging)** mode (`PRAGMA journal_mode = WAL;`) for concurrent read/write transactions.
  * Per-thread database connection pooling eliminating cross-thread database lockouts.
  * Versioned schema migrations preserving database integrity across app updates.
* **Instant Search & Dynamic Filtering:**
  * Instant real-time search across Title, Artist, Album, and Genre.
  * Dedicated views for **Favorites**, **Recently Added**, and **Recently Played**.

---

## 4. Metadata & Album Artwork Engine

* **Format-Aware Artwork Extraction:**
  * Native binary extraction from FLAC Picture Block Type 6 (front cover).
  * ID3v2.3 / ID3v2.4 APIC tag extraction for MP3, WAV, and AIFF.
  * Native background `QProcess` integration for universal FFprobe metadata and FFmpeg cover extraction without flashing console windows.
* **Persistent Disk Cache:**
  * Extracted covers are hashed and cached to `%LocalAppData%/SONIQ/artwork_cache/`.
  * Instant cover rendering on subsequent launches without re-parsing audio files.
* **Technical Audio Inspector:**
  * In-depth audio telemetry display showing container format, codec, sample rate, bit depth, channel layout, bitrate, file size, and bit-perfect eligibility status.

---

## 5. Studio-Grade 64-Bit DSP & Equalizer

* **10-Band Graphic Equalizer:**
  * ISO standard octave center frequencies: `31 Hz`, `62 Hz`, `125 Hz`, `250 Hz`, `500 Hz`, `1 kHz`, `2 kHz`, `4 kHz`, `8 kHz`, `16 kHz`.
  * Smooth per-band gain adjustment from `-12 dB` to `+12 dB`.
* **31-Band 1/3-Octave Graphic Equalizer:**
  * Surgical acoustic frequency correction for room acoustics and headphone equalization profiles.
* **Parametric Equalizer:**
  * Second-order biquad filter topologies: Peaking, Low Shelf, High Shelf, High Pass, Low Pass, and Notch with arbitrary Q factor.
* **Acoustic Presets:**
  * Bundled presets: *Flat, Bass Boost, Treble Boost, Vocal Clarity, Electronic, Classical, Rock, Acoustic, and Audiophile Reference*.
* **Bauer Stereophonic-to-Binaural Crossfeed (BS2B):**
  * Frequency-shaped spatial crossfeed between stereo channels.
  * Eliminates extreme left/right headphone isolation fatigue and recreates natural loudspeaker spatial imaging.
* **True-Peak Limiter & Anti-Clipping:**
  * Lookahead peak limiter prevents digital inter-sample clipping while preserving full acoustic dynamic range.
* **Bit-Perfect Integrity Audit:**
  * Real-time engine telemetry that audits whether the output stream remains bit-perfect or is mathematically altered by volume, DSP, or resampling.

---

## 6. Modern Desktop User Interface (Qt Quick / QML)

* **Intelligent Window Geometry Clamping:**
  * Proportional geometry calculation that automatically centers the application window within the active monitor's available work area on startup, preventing taskbar overlap on any screen resolution (from 720p/768p up to 4K).
* **Dark Audiophile Theme:**
  * Custom dark design system featuring vibrant crimson accents (`#E63946`), glassmorphism cards, and fluid 60 FPS animations.
* **Persistent Bottom Player Bar:**
  * Embedded cover artwork thumbnail and scrolling track title/artist.
  * Real-time stereo audio spectrum visualizer responding dynamically to peak levels.
  * Scrubbing seek timeline with elapsed and remaining duration timers.
  * Transport controls: Play/Pause, Next, Previous, Shuffle, and Repeat (Off, Repeat All, Repeat One).
  * Interactive volume slider with instant mute toggle.
* **Slide-in Playing Queue Drawer:**
  * Smooth right-hand drawer displaying upcoming queue tracks.
  * Reorder, remove, or clear queue tracks on the fly.

---

## 7. Windows Standalone & Portable Deployment

* **Native Windows GUI Executable:**
  * Compiled with the `WIN32` subsystem flag (`/SUBSYSTEM:WINDOWS`), ensuring zero command prompt or console window popups.
* **Zero Runtime DLL Dependencies:**
  * Modern C++20 and stack unwind runtimes statically linked (`-static-libstdc++ -static-libgcc`).
  * Runs on any Windows 10 or 11 system without missing DLL errors.
* **Dual Execution Modes:**
  * **Direct Root Launcher:** Run `soniq_app.exe` directly in the project root.
  * **Portable Distribution Bundle:** Standalone `dist/` directory that can be transferred to a USB drive or secondary PC and run immediately without installation.
