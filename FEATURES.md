# SONIQ — Application Features

> **Pure Sound. Infinite Possibilities.**  
> An offline-first, bit-perfect, cross-platform audiophile music player built with C++20 and Qt 6.

---

## 1. Audiophile Audio Playback Engine

* **Bit-Perfect Hardware Output (WASAPI Exclusive Mode):**
  * Direct kernel streaming bypassing the Windows Audio Engine, CAudioLimiter, and system mixer (`kmixer`).
  * Delivers untampered, bit-exact audio samples directly to your external DAC or audio interface.
  * Automatic graceful fallback to WASAPI Shared Mode if another application locks the hardware endpoint.

* **Native DSD & DoP Bitstream Engine:**
  * Direct Stream Digital playback for `.dsf` and `.dff` (Super Audio CD) bitstream files.
  * **DoP v1.1 Unpacker (DSD over PCM):** Formats 1-bit DSD data into standard 16-bit PCM frames with alternating `0x05` and `0xFA` sync markers at 176.4 kHz (DSD64) or 352.8 kHz (DSD128) for hardware DoP DACs.
  * **Linear-Phase Decimation Filter:** High-order 64-tap linear-phase FIR decimation filter with a 40 kHz cutoff for pristine software rendering of DSD on standard PCM soundcards.

* **Gapless Transitions & Lookahead Pre-Buffering:**
  * Background lookahead pre-decoder caches the upcoming track into ring buffers before the current track completes.
  * Seamless zero-delay track boundary handovers with zero output device re-open stutter.
  * Configurable constant-acoustic-power (equal-power) crossfading from 0 to 10 seconds.

* **High-Resolution Master Audio Support:**
  * Native playback of sampling rates up to **768 kHz** and bit depths up to **32-bit floating point**.

---

## 2. Universal Format Compatibility

| Category | Formats & Codecs | Bit Depth & Sample Rates |
|---|---|---|
| **Uncompressed PCM** | WAV, AIFF | 8-bit to 32-bit float, up to 768 kHz |
| **Lossless Compression** | FLAC, ALAC (Apple Lossless), WavPack, APE (Monkey's Audio) | Up to 24-bit / 192 kHz and 32-bit float |
| **Direct Stream Digital** | DSF, DFF (DSD64, DSD128, DSD256) | 2.8224 MHz to 11.2896 MHz 1-bit |
| **Lossy Codecs** | MP3, AAC, OGG Vorbis, Opus | 32 kbps to 320 kbps, CBR/VBR, up to 48 kHz |

---

## 3. Format-Aware Artwork Extraction & Caching

* **Embedded Metadata Parsing:**
  * Native binary extraction of embedded cover art from FLAC Picture Block Type 6 (front cover).
  * ID3v2.3 / ID3v2.4 APIC tag extraction for MP3, AIFF, and WAV.
  * Vorbis comment artwork parsing for OGG and Opus.
* **Persistent SHA-256 Disk Cache:**
  * Decoded album artwork is cached in `%LocalAppData%/SONIQ/artwork_cache/` using file-content hash keys.
  * Instant cover rendering on subsequent application launches without disk re-reading.

---

## 4. Studio-Grade 64-Bit DSP Pipeline

* **10-Band Graphic Equalizer:**
  * Standard ISO octave center frequencies: 31 Hz, 62 Hz, 125 Hz, 250 Hz, 500 Hz, 1 kHz, 2 kHz, 4 kHz, 8 kHz, 16 kHz.
  * Continuous per-band gain adjustment from -12 dB to +12 dB.
* **31-Band 1/3-Octave Graphic Equalizer:**
  * High-precision acoustic compensation for room modes and headphone response curves.
* **Parametric Equalizer:**
  * Second-order biquad filter topologies: Peaking, Low Shelf, High Shelf, High Pass, Low Pass, and Notch with arbitrary Q factor.
* **Acoustic Presets:**
  * Bundled curated profiles: *Flat, Bass Boost, Treble Boost, Vocal Clarity, Electronic, Classical, Rock, Acoustic, and Audiophile Reference*.
* **Bauer Stereophonic-to-Binaural Crossfeed (BS2B):**
  * Blends subtle delay and frequency-shaped crossfeed between stereo channels.
  * Eliminates extreme left/right headphone isolation and listening fatigue, simulating natural loudspeaker stereo imaging.
* **True-Peak Limiter & Transparent Anti-Clipping:**
  * Lookahead peak-limiting prevents digital inter-sample clipping while preserving dynamic range.
* **Bit-Perfect Integrity Audit:**
  * Real-time engine telemetry that verifies whether the audio stream is currently bit-perfect or mathematically altered by DSP, volume controls, or resampling.

---

## 5. Modern Desktop User Interface (Qt Quick / QML)

* **Responsive Design & Monitor Fitting:**
  * Proportional geometry clamping that automatically centers the window on startup within the available monitor work area, keeping clean breathing room above the Windows taskbar.
  * Resilient responsive breakpoints supporting displays from 720p/768p laptops up to 4K high-DPI monitors.
* **Dark Audiophile Theme:**
  * Custom dark design system with vibrant crimson accents (`#E63946`), glassmorphism cards, and smooth motion curves.
* **Persistent Mini-Player (Bottom Bar):**
  * Embedded cover artwork thumbnail and scrolling track title/artist.
  * Real-time stereo audio spectrum visualizer reacting to peak audio levels.
  * Scrubbing seek timeline with elapsed and remaining duration timers.
  * Transport controls: Play/Pause, Next, Previous, Shuffle, and Repeat (Off, Repeat All, Repeat One).
  * Interactive volume slider with instant mute toggle.
* **Slide-in Playing Queue Drawer:**
  * Smooth right-hand drawer listing all upcoming tracks in the active queue.
  * Reorder, remove, or clear queue on the fly.
* **Audio Inspector Screen:**
  * In-depth technical telemetry for any track: container format, codec, sample rate, bit depth, channel layout, bitrate, file size, and bit-perfect eligibility status.

---

## 6. Offline-First Library & Database Management

* **Zero Tracking & Privacy First:**
  * No accounts, no subscriptions, and zero online telemetry. All data stays local to your machine.
* **Asynchronous Recursive Library Scanner:**
  * Multithreaded background directory scanner that indexes large music collections without dropping UI frame rates.
  * Live scanning status and progress tracking.
* **SQLite Persistence (Schema Migrations):**
  * Local high-performance SQLite database storing library tracks, playlists, playback statistics, and user preferences.
  * Automatic versioned migrations ensuring schema integrity across updates.
* **Instant Search & Dynamic Filtering:**
  * Instant search across Title, Artist, Album, and Genre.
  * One-click views for *Favorites*, *Recently Added*, and *Recently Played*.

---

## 7. Windows Standalone & Portable Deployment

* **Native Windows GUI Executable:**
  * Compiled with the `WIN32` subsystem flag (`/SUBSYSTEM:WINDOWS`), ensuring zero console or command prompt window popups.
* **Zero Runtime DLL Dependency:**
  * Modern C++20 and stack unwind runtimes statically linked (`-static-libstdc++ -static-libgcc`).
  * Will not crash with missing `libc++.dll` or `libunwind.dll` errors on any Windows 10 or 11 system.
* **Dual Execution Modes:**
  * **Direct Root Launcher:** Double-click `soniq_app.exe` in the project root folder.
  * **Portable Distribution Bundle:** Standalone `dist/` directory that can be copied to a USB drive or secondary PC and run immediately without installation.
