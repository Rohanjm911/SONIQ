# SONIQ Architecture & Design Specification

## Overview

SONIQ is architected around strict boundaries between the **Audio Engine Thread**, **Background Worker Threads**, and the **Qt Quick / QML UI Thread**. Audio correctness, buffer latency, and bit-perfect reproduction are never compromised by user interface animation frame rates or asynchronous library operations.

---

## Logical Layer Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                   QML Presentation Layer                         │
│  (NavigationSidebar, MiniPlayer, QueueDrawer, NowPlaying, etc.)  │
└─────────────────────────────────▲────────────────────────────────┘
                                  │ Qt Signals / Properties
┌─────────────────────────────────▼────────────────────────────────┐
│                   Application Services Layer                     │
│  • PlaybackController    • LibraryModel     • DspService         │
│  • SettingsService       • AudioInspectorService                 │
└───────────────────▲──────────────────────────────▲───────────────┘
                    │                              │
┌───────────────────▼──────────────────┐   ┌───────▼───────────────┐
│        Audio Subsystem               │   │   Database & Library  │
│  • AudioPlaybackEngine (Worker Thread)│   │  • DatabaseManager    │
│  • PlaybackQueue (Thread-Safe)       │   │  • LibraryScanner     │
│  • DspPipeline                       │   │  • AudioProbe         │
│    - BiquadFilter (Direct Form II)   │   └───────────────────────┘
│    - 10-Band / 31-Band Equalizer     │
│    - Lookahead Peak Limiter          │
│    - Bauer Binaural Crossfeed        │
│  • AudioDecoder (WAV & FFmpeg pipe)  │
│  • IAudioOutput (WASAPI Exclusive)   │
└──────────────────────────────────────┘
```

---

## Key Subsystems

### 1. Audio Processing Pipeline
1. **Decoder**: Reads chunks (default 1024 frames) of uncompressed 32-bit floating point PCM audio from uncompressed audio streams or FFmpeg process pipes.
2. **DSP Pipeline**:
   - Master Preamp Gain application
   - ReplayGain loudness normalization (when enabled)
   - Biquad Cascaded Filter Banks (Equalizer)
   - Stereo Balance & Width matrix transformation
   - Bauer Crossfeed convolution
   - Peak Limiter & Soft-Knee Anti-Clipping prevention
3. **Telemetry & Visualizer Hook**: Extracts waveform blocks and peak levels for non-blocking QML visualizers.
4. **Hardware Output**: Writes processed audio to Windows Core Audio (`IAudioClient`) buffer in event-driven or polling mode.

### 2. Bit-Perfect Verification Engine
A lossless file does not guarantee bit-perfect reproduction if software volume, resampling, or DSP filters modify sample values. SONIQ continuously audits the active playback graph:
- **Verified**: When WASAPI Exclusive Mode is locked, hardware sample rate equals source sample rate, volume is 100%, and all DSP stages are bypassed.
- **Available**: Hardware supports the stream format natively, but shared-mode or attenuation is active.
- **Unavailable**: Active DSP (e.g., EQ gain != 0, Crossfeed on, Limiter active) or software volume < 100% is modifying sample values. Reasons are enumerated and reported to the user in the UI.

### 3. Local Persistence & Aggregations
- Implemented with SQLite using versioned migrations (`schema_version`).
- Tracks, playlists, playback statistics, and user preferences are indexed locally.
- High-performance distinct group-by aggregations (`AlbumSummary`, `ArtistSummary`, `GenreSummary`) allow instant client-side filtering without heavy table scans.
- Library scanner iterates folders recursively on a background worker thread with zero UI lag and updates the library model via thread-safe signals.

### 4. Real-Time Telemetry & Synchronized Lyrics Engine
- **Visualizer Telemetry Hook**: Exposes a real-time 128-sample normalized waveform array (`PlaybackController::waveform()`) and Left/Right peak amplitudes to QML via Q_PROPERTY without blocking the audio callback.
- **Synchronized Lyrics Parser**: Dynamically locates adjacent `.lrc` and `.txt` files matching the playing track or folder stem, parses millisecond timestamps (`[mm:ss.xx]`), and emits time-synced line indices for smooth auto-scrolling lyrics presentation.

### 5. UI Architecture & Responsive Geometry
- Replaces standard Qt Quick `ScrollView` with explicit `Flickable` viewports to avoid circular geometry binding loops on high-DPI displays.
- Strict proportional anchoring and bounded text eliding ensure zero column squishing or text collision across arbitrary window sizes.
