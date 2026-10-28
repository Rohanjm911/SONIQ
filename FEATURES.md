# SONIQ — Comprehensive Feature Specification & Architecture

> **Pure Sound. Infinite Possibilities.**  
> An offline-first, bit-perfect, cross-platform audiophile music player built with **C++20** and **Qt 6.8.2 Quick/QML**.

---

## Table of Contents
1. [Audiophile Playback Engine](#1-audiophile-playback-engine)
2. [Native DSD & DoP Stream Processing](#2-native-dsd--dop-stream-processing)
3. [Studio-Grade 64-Bit DSP & Equalizer Pipeline](#3-studio-grade-64-bit-dsp--equalizer-pipeline)
4. [Universal Format & Container Compatibility](#4-universal-format--container-compatibility)
5. [High-Performance Multi-Threaded Library Scanner](#5-high-performance-multi-threaded-library-scanner)
6. [Interactive Multi-Criteria Library Sorting](#6-interactive-multi-criteria-library-sorting)
7. [Custom Playlists & Queue Management](#7-custom-playlists--queue-management)
8. [Built-In Audio Tag & Metadata Editor](#8-built-in-audio-tag--metadata-editor)
9. [Format-Aware Artwork Extraction & Cache](#9-format-aware-artwork-extraction--cache)
10. [Technical Audio Inspector & Bit-Perfect Audit](#10-technical-audio-inspector--bit-perfect-audit)
11. [Modern Desktop UI & Design Tokens](#11-modern-desktop-ui--design-tokens)
12. [Animated Splash Screen & Fluid Transitions](#12-animated-splash-screen--fluid-transitions)
13. [Silent Process Execution (Zero CMD Windows)](#13-silent-process-execution-zero-cmd-windows)
14. [Windows Standalone & Portable Deployment](#14-windows-standalone--portable-deployment)

---

## 1. Audiophile Playback Engine

* **Bit-Perfect Hardware Output (WASAPI Exclusive Mode):**
  * Direct kernel-level audio streaming that bypasses the Windows Audio Engine (`audiodg.exe`), `CAudioLimiter`, and system mixer (`kmixer`).
  * Delivers bit-exact, unaltered audio samples directly to your external USB DAC, sound card, or audio interface without intermediate operating system resampling or dithering.
  * Automatic graceful fallback to **WASAPI Shared Mode** if another application or device holds exclusive access, with automatic channel/format conversion.
* **Dedicated High-Priority Audio Threading:**
  * Engine playback, decoding, lookahead ring buffers, and signal DSP run completely decoupled on an independent, high-priority thread.
  * Guarantees zero audio dropouts, clicks, or buffer under-runs even during intensive background folder scans, window resizing, or complex QML animations.
* **Lookahead Pre-Buffering & Seamless Gapless Transitions:**
  * Multi-threaded lookahead pre-decoder pre-buffers the upcoming track into ring buffers before the active track finishes.
  * Eliminates silent gaps between tracks, creating an unbroken musical flow for live concerts, classical movements, and continuous-mix electronic albums.
  * Zero-restart audio endpoint reuse avoids hardware DAC relay clicks between consecutive tracks of matching sample rate and bit depth.
* **Equal-Power Constant-Acoustic-Power Crossfade:**
  * Optional trigonometric equal-power crossfader ($\cos / \sin$ curve) configurable from `0.0s` (strict gapless) up to `10.0s`.
  * Eliminates acoustic volume dips during transitions by maintaining consistent perceived loudness across track overlaps.
* **Ultra-High Resolution Audio Support:**
  * Supports native audio streams from standard Red Book CD (44.1 kHz / 16-bit) up to **768 kHz / 32-bit floating point** and high-rate integer containers.

---

## 2. Native DSD & DoP Stream Processing

```
                       ┌───────────────────────────────┐
                       │   .dsf / .dff Audio Stream    │
                       └──────────────┬────────────────┘
                                      │
                         Is DoP Hardware DAC Enabled?
                                      │
                     ┌────────────────┴────────────────┐
                     ▼ YES                             ▼ NO
       ┌──────────────────────────────┐  ┌──────────────────────────────┐
       │     DoP v1.1 Unpacker        │  │ 64-Tap Linear-Phase Decimator│
       │   16-Bit PCM Container       │  │ (40 kHz Cutoff FIR Filter)   │
       │ Markers 0x05 / 0xFA (176.4k) │  │  Convert 1-Bit DSD to PCM    │
       └──────────────┬───────────────┘  └──────────────┬───────────────┘
                      ▼                                 ▼
       ┌──────────────────────────────┐  ┌──────────────────────────────┐
       │ Bit-Perfect Hardware Output  │  │   Studio 64-Bit DSP Stream   │
       └──────────────────────────────┘  └──────────────────────────────┘
```

* **Direct Stream Digital (DSD) Parsing:**
  * Direct low-latency binary stream extraction for `.dsf` and `.dff` (Super Audio CD / SACD) bitstream audio files.
  * Supports DSD64 (2.8224 MHz), DSD128 (5.6448 MHz), and DSD256 (11.2896 MHz).
* **DoP v1.1 Framing (DSD over PCM):**
  * Formats raw 1-bit DSD bitstream bytes into standard 16-bit PCM frames with alternating `0x05` and `0xFA` synchronization markers.
  * Transmits bitstreams at 176.4 kHz (DSD64) or 352.8 kHz (DSD128) over standard USB Audio Class 2.0 endpoints directly into hardware DoP-compatible DACs.
* **Linear-Phase FIR Decimation Filter:**
  * High-order 64-tap linear-phase FIR decimation filter with a 40 kHz cutoff frequency for software conversion of 1-bit DSD to PCM.
  * Enables playback of high-res SACD / DSD rips on standard sound cards and headphones without high-frequency quantization noise.

---

## 3. Studio-Grade 64-Bit DSP & Equalizer Pipeline

* **10-Band Graphic Equalizer (ISO Standard Octave Bands):**
  * ISO center frequencies: `31 Hz`, `62 Hz`, `125 Hz`, `250 Hz`, `500 Hz`, `1 kHz`, `2 kHz`, `4 kHz`, `8 kHz`, and `16 kHz`.
  * Continuous per-band gain adjustment from `-12.0 dB` to `+12.0 dB` with fine granularity.
  * Real-time interactive response curve visualization dynamically rendered using responsive cubic Bezier curves in QML.
* **31-Band 1/3-Octave Graphic Equalizer:**
  * Surgical acoustic frequency correction for room acoustics and headphone equalization profiles.
* **Parametric Equalizer:**
  * Second-order biquad filter topologies: Peaking, Low Shelf, High Shelf, High Pass, Low Pass, and Notch with arbitrary Q factor.
* **Acoustic Presets:**
  * Curated built-in presets: *Flat*, *Bass Boost*, *Treble Boost*, *Vocal Clarity*, *Electronic*, *Classical*, *Rock*, *Acoustic*, and *Audiophile Reference*.
  * Instant preset switching with animated slider transitions.
* **Bauer Stereophonic-to-Binaural Crossfeed (BS2B):**
  * Frequency-shaped spatial crossfeed between left and right stereo channels.
  * Eliminates extreme left/right headphone isolation fatigue and recreates natural loudspeaker spatial imaging.
* **True-Peak Limiter & Anti-Clipping:**
  * Lookahead peak limiter prevents digital inter-sample clipping while preserving full acoustic dynamic range.
* **Bit-Perfect Integrity Audit:**
  * Real-time engine telemetry that audits whether the output stream remains bit-perfect or is mathematically altered by volume, DSP, or resampling.
  * Shows exact status badges in UI: **BIT-PERFECT DIRECT** or **DSP ACTIVE (MODIFIED)** with detailed explanations.

---

## 4. Universal Format & Container Compatibility

SONIQ supports virtually all uncompressed, lossless, DSD, and lossy audio containers:

| Category | Formats & Extensions | Bit Depth & Sample Rates | Decoding Pipeline |
|---|---|---|---|
| **Uncompressed PCM** | `.wav`, `.wave`, `.aiff`, `.aif` | 8-bit to 32-bit integer & float, up to 768 kHz | Direct RIFF/AIFF Binary Parser (Zero Latency) |
| **Lossless Compression** | `.flac`, `.alac`, `.m4a`, `.wv` (WavPack), `.ape` (Monkey's Audio), `.tta`, `.tak` | Up to 24-bit / 192 kHz and 32-bit float | High-Throughput Universal Stream Pipeline |
| **Direct Stream Digital** | `.dsf`, `.dff` (DSD64, DSD128, DSD256) | 2.8224 MHz to 11.2896 MHz 1-bit | Native DoP v1.1 Unpacker / 64-Tap FIR Decimator |
| **Lossy Codecs** | `.mp3`, `.aac`, `.ogg` (Vorbis), `.opus`, `.wma`, `.ac3`, `.dts` | 32 kbps to 320 kbps, CBR/VBR, up to 48 kHz | Floating-point PCM Stream Decoder |

---

## 5. High-Performance Multi-Threaded Library Scanner

* **Asynchronous Background Indexing:**
  * Multi-threaded background scanner runs on an independent worker thread without dropping GUI frame rates.
  * Handles large multi-gigabyte collections and deeply nested directory trees.
* **Single Folder Replacement Mode:**
  * When scanning a music folder, SONIQ automatically clears out obsolete entries from previously scanned folders, keeping your library cleanly focused on your current selection without orphaned files.
* **Full Unicode & Path Normalization:**
  * Comprehensive Unicode, percent-encoding (`%20`), and Windows path spaces handling.
  * Seamlessly resolves drives, folders, and tracks with non-ASCII or international characters.
* **Live Scanning Progress Banner:**
  * Real-time progress notifications in the top UI banner displaying `(X / Y)` scanned file counts, percentage completion, and current directory path.
* **Thread-Safe SQLite Database with WAL Mode:**
  * Embedded SQLite database running in **WAL (Write-Ahead Logging)** mode (`PRAGMA journal_mode = WAL;`) for concurrent read/write transactions.
  * Per-thread database connection pooling eliminating cross-thread database lockouts.
  * Versioned schema migrations (v1 base schema, v2 artwork persistence) preserving database integrity across app updates.

---

## 6. Interactive Multi-Criteria Library Sorting

* **Interactive Column Headers:**
  * Clickable column headers in the track list: **Title**, **Artist**, **Album**, **Duration**, and **Format**.
  * Toggles ascending (`▲`) and descending (`▼`) sort order with instant animated list re-ordering.
* **Dedicated Sort Dropdown Menu:**
  * Quick-access dropdown menu offering 10 sorting algorithms:
    * **Title (A → Z)** / **Title (Z → A)**
    * **Artist (A → Z)** / **Artist (Z → A)**
    * **Album (A → Z)** / **Album (Z → A)**
    * **Duration (Shortest First)** / **Duration (Longest First)**
    * **Date Added (Newest First)** / **Date Added (Oldest First)**
    * **Format (Lossless First)** / **Format (Lossy First)**
    * **Favorites First**
    * **Most Played** / **Least Played**
    * **Track Number (Disc & Track Sequence)**
    * **Shuffled (Randomized Library View)**

---

## 7. Custom Playlists & Queue Management

* **Create & Manage Custom Playlists:**
  * Built-in `CreatePlaylistDialog` allows instantaneous playlist creation with custom names.
  * Dedicated Playlists drawer in the navigation sidebar displaying track counts and creation dates.
* **Contextual Track Actions Menu (`•••`):**
  * Every track row features an interactive options popup with actions:
    * **Add to Playlist**: Pops up `AddToPlaylistDialog` to select or create a destination playlist.
    * **Edit Tags**: Launches the built-in metadata tag editor modal.
    * **Toggle Favorite**: Instantly adds or removes track from Favorites collection.
* **Slide-In Queue Drawer:**
  * Slide-in right panel displaying the live play queue.
  * Supports clear queue, reordering, and direct track jump.

---

## 8. Built-In Audio Tag & Metadata Editor

* **In-App Modal Tag Editor (`TagEditorDialog.qml`):**
  * Clean, dark-mode dialog allowing direct metadata editing without leaving the music player.
  * Editable fields: **Title**, **Artist**, **Album**, **Year**, **Genre**, and **Track Number**.
* **Dual Persistence Layer:**
  * Immediately updates the in-memory `LibraryModel` and persists changes to the SQLite database.
  * Synchronously updates display titles, artist credits, and album categorization in real time.

---

## 9. Format-Aware Artwork Extraction & Cache

* **Multi-Format Embedded Artwork Extraction:**
  * Native binary extraction from FLAC Picture Block Type 6 (front cover).
  * ID3v2.3 / ID3v2.4 APIC tag extraction for MP3, WAV, and AIFF.
  * Automatic fallback to folder-level artwork files (`cover.jpg`, `folder.png`, `front.jpg`).
* **Persistent Disk Cache:**
  * Extracted covers are hashed using SHA-256 and stored in `%AppData%/SONIQ/SONIQ/artwork_cache/`.
  * Instant cover rendering on subsequent launches without re-parsing audio files.
* **Ambilight Aura Glow:**
  * UI dynamically samples album art accent colors to project a subtle, glowing ambient backdrop on the Now Playing screen.

---

## 10. Technical Audio Inspector & Bit-Perfect Audit

* **In-Depth Audio Telemetry Display:**
  * Real-time metadata inspection panel showing:
    * **Container & Format**: FLAC, DSF, WAV, ALAC, MP3, etc.
    * **Audio Codec & Profile**: PCM integer, PCM 32-bit float, DSD 1-bit bitstream.
    * **Sample Rate**: 44.1 kHz up to 768 kHz, with Hi-Res Gold Badges for streams $\ge 96\text{ kHz}$.
    * **Bit Depth**: 16-bit, 24-bit, 32-bit float, or 1-bit DSD.
    * **Channel Configuration**: Stereo 2.0, Mono 1.0, 5.1 Surround.
    * **Bitrate & Compression Ratio**: Exact kbps calculations.
* **Live Bit-Perfect Verification Badge:**
  * High-visibility status indicator showing cyan **VERIFIED BIT-PERFECT** when audio samples reach the DAC with zero alteration, or warning badge with active modification reasons (EQ enabled, volume attenuation, crossfeed active).

---

## 11. Modern Desktop UI & Design Tokens

* **Unified Modern Design System:**
  * Comprehensive design token architecture in `Colors.qml`, `Typography.qml`, `Motion.qml`, and `Theme.qml`.
  * Fluid 60 FPS transitions and ease-in-out animations for all drawers, sliders, and views.
* **Dynamic Accent Color Picker:**
  * Choose between 5 curated audiophile accent themes in Settings:
    * 🔴 **Apple Crimson** (`#FA2D48`)
    * 🔵 **Neon Cyan** (`#00F0FF`)
    * 🟢 **Emerald Green** (`#10B981`)
    * 🟡 **Amber Gold** (`#F59E0B`)
    * 🟣 **Royal Purple** (`#8B5CF6`)
* **Dark & Light Mode Switching:**
  * Supports seamless switching between deep OLED dark mode (`#0A0B0E`) and clean daylight mode (`#F8F9FA`), persisted in user preferences.
* **Full-Screen (F11) & Responsive Monitor Clamping:**
  * Adaptive layout scaling that gracefully handles resolutions from 720p/768p up to 4K widescreen.
  * Intelligent monitor work-area geometry calculation centering the window on startup without taskbar overlap.
  * Instant fullscreen toggle via keyboard shortcut (**F11**).

---

## 12. Animated Splash Screen & Fluid Transitions

* **Cinematic Startup Experience:**
  * Fullscreen dark cinematic overlay (`#0A0B0E`) playing a smooth animated introduction for 6 seconds.
  * Features high-frame rate animated GIF playback with a gentle scale-in and fade-in entrance.
* **Pulsing Accent Ring Glow:**
  * Subtle accent-colored pulsating ring orbiting behind the logo during initialization.
* **Smooth Cubic Fade-Out:**
  * After 6 seconds, executes an 800ms smooth cubic fade-out transition, seamlessly unveiling the main music player.
* **Input-Absorbing Barrier with Click-to-Skip:**
  * Transparent `MouseArea` blocks accidental clicks from bleeding through to underlying controls during splash.
  * Clicking anywhere during the splash gracefully skips the remainder and triggers immediate fade-out.

---

## 13. Silent Process Execution (Zero CMD Windows)

* **Win32 `CREATE_NO_WINDOW` Architecture:**
  * All background helper utilities (FFprobe metadata readers, artwork extractors) execute via native Win32 `CreateProcessW` APIs with the `CREATE_NO_WINDOW` flag.
  * Standard output and error streams are redirected to anonymous in-memory pipes.
* **Zero Console Flashes:**
  * Eliminates annoying black command prompt (`cmd.exe`) window flashes that plague typical Windows media player utilities when switching songs or scanning folders.

---

## 14. Windows Standalone & Portable Deployment

* **Native Windows GUI Executable:**
  * Compiled with the `WIN32` subsystem flag (`/SUBSYSTEM:WINDOWS`), ensuring a purely graphical desktop application.
* **Statically Linked C++ Runtime:**
  * Modern C++20 and stack unwind runtimes statically linked (`-static-libstdc++ -static-libgcc`).
  * Runs on any Windows 10 or 11 system without missing MSVCP/MinGW runtime DLL errors.
* **Dual Execution Modes:**
  * **Direct Root Launcher:** Run `soniq_app.exe` directly from the project root.
  * **Portable Distribution Bundle:** Standalone `dist/` directory that can be transferred to a USB drive or secondary PC and run immediately without installation.
