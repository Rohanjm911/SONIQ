# SONIQ Audio Engine Documentation

## Core Audio Philosophy

1. **Precision**: All internal audio computations and IIR filter calculations are executed in double-precision 64-bit floating point arithmetic.
2. **Honesty**: Bit-perfect status is verified against actual device state, output mode, and DSP activity rather than fabricated from file extension names.
3. **Decoupled Performance**: Critical audio buffers are fed continuously by a dedicated high-priority audio worker thread (`AudioPlaybackEngine::audioThreadLoop()`).

---

## Output Backends

### Windows Core Audio (WASAPI)
- Uses `IMMDeviceEnumerator` to detect hardware render endpoints.
- **Exclusive Mode (`AUDCLNT_SHAREMODE_EXCLUSIVE`)**:
  - Direct hardware access without OS mixing, sample rate conversion, or APO dithering.
  - Negotiates native device clock frequencies (44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz, 176.4 kHz, 192 kHz, 352.8 kHz, 384 kHz).
- **Shared Mode (`AUDCLNT_SHAREMODE_SHARED`)**:
  - Safe fallback when exclusive access is rejected by drivers or when multiple applications share audio output.

### In-Memory Virtual Sink (`NullAudioOutput`)
- Used for automated testing and CI verification.
- Stores output frames in memory buffers for exact sample-by-sample bit verification.

---

## Digital Signal Processing (DSP)

### Biquad Filter Design
Implemented using the **Direct Form II Transposed** canonical structure:
```
y[n] = b0 * x[n] + z1[n-1]
z1[n] = b1 * x[n] - a1 * y[n] + z2[n-1]
z2[n] = b2 * x[n] - a2 * y[n]
```
Advantages:
- Numerically superior stability compared to Direct Form I.
- Resists limit-cycle oscillations during transient steps.
- Normalized by $a_0$ to guarantee single-cycle multiplication and addition.

### 10-Band Graphic Equalizer
Standard ISO Octave Center Frequencies:
- Band 0: 31.25 Hz (Low Shelf)
- Band 1: 62.5 Hz (Peaking EQ, Q = 1.414)
- Band 2: 125 Hz (Peaking EQ, Q = 1.414)
- Band 3: 250 Hz (Peaking EQ, Q = 1.414)
- Band 4: 500 Hz (Peaking EQ, Q = 1.414)
- Band 5: 1 kHz (Peaking EQ, Q = 1.414)
- Band 6: 2 kHz (Peaking EQ, Q = 1.414)
- Band 7: 4 kHz (Peaking EQ, Q = 1.414)
- Band 8: 8 kHz (Peaking EQ, Q = 1.414)
- Band 9: 16 kHz (High Shelf)

### Soft-Knee Peak Limiter
When EQ bands or preamps are boosted, signal peaks can exceed $\pm 1.0$ (0 dBFS). The limiter calculates the instantaneous frame peak and engages a smooth exponential release envelope:
$$targetGain = \frac{compressedPeak}{framePeak}$$
This guarantees zero inter-sample clipping while preserving transient clarity.

### Bauer Binaural Crossfeed
Simulates speaker acoustic cross-talk for headphone listening:
- Cutoff frequency: 700 Hz
- Feed level: -4.5 dB
- Eliminates extreme stereo separation fatigue on vintage master recordings.

---

## Native Direct Stream Digital (DSD) & DoP v1.1

SONIQ includes dedicated decoders and processors for `.dsf` and `.dff` (DSDIFF) high-resolution audio files:

### 1. DSD over PCM (DoP v1.1 Standard)
- For native bit-perfect playback to DoP-compatible USB DACs and WASAPI Exclusive endpoints.
- Each 16 consecutive DSD bits are packed into the lower 16 bits of a 24-bit PCM word.
- The upper 8 bits carry an alternating marker sequence (`0x05` and `0xFA`) across successive audio frames.
- Clock rate:
  - DSD64: $2,822,400 / 16 = 176.4\text{ kHz}$ 24-bit PCM
  - DSD128: $5,644,800 / 16 = 352.8\text{ kHz}$ 24-bit PCM
- Transmitted without any digital attenuation, volume scaling, or DSP modification, preserving the 1-bit PDM stream.

### 2. High-Precision FIR Decimation (DSD-to-PCM)
- For DACs operating in shared mode or when DSP (equalization, crossfeed, limiter) is engaged.
- Converts 1-bit DSD bitstream into 32-bit float PCM at 88.2 kHz or 176.4 kHz.
- 64-tap symmetric linear-phase Blackman-windowed FIR low-pass filter with 40 kHz cutoff frequency.
- Completely filters out ultrasonic sigma-delta 1-bit quantization noise while preserving pure analog micro-dynamics.

---

## Gapless Playback & Lookahead Buffering

Seamless audio track transitions without clicks, pops, or silence gaps:

### 1. Lookahead Pre-Buffering
- When the active track reaches its final 4.0 seconds, the engine queries the queue (`PlaybackQueue::peekNext()`).
- Asynchronously pre-instantiates and pre-opens the next `AudioDecoder` on a background worker thread.
- Decodes the initial 4096 frames into an internal lookahead FIFO buffer, ready for immediate playback.

### 2. Seamless Output Endpoint Handover
- When the current track reaches EOF, the engine compares the incoming track's sample rate and channel layout.
- If identical, the active WASAPI output session is **not closed or reopened**.
- Decoders are swapped atomically: `m_decoder = std::move(m_lookaheadDecoder);`
- Playback continues seamlessly with zero inter-track latency and 0 zero-byte gaps.

### 3. Equal-Power Crossfading
- When crossfading is enabled in Settings, the engine applies constant-acoustic-power sinusoidal curves:
  $$g_{\text{out}}(t) = \cos\left(\frac{\pi}{2} \cdot \alpha\right), \quad g_{\text{in}}(t) = \sin\left(\frac{\pi}{2} \cdot \alpha\right)$$
  where $\alpha = 1.0 - \frac{t_{\text{remaining}}}{T_{\text{crossfade}}}$.
- Prevents volume dips in the transition region. Bit-perfect indicator automatically reports `Unavailable` with the reason `"Equal-power crossfading active"`.

---

## Format-Aware Embedded Artwork Caching

- Extractors:
  - FLAC: Native binary parser for `METADATA_BLOCK_PICTURE` (type 6).
  - MP3: Native ID3v2.3 / ID3v2.4 synchsafe parser for `APIC` frames.
  - Universal Fallback: Zero-reencode stream copy cover extractor.
- Disk Cache: Persistent storage under `%LocalAppData%/SONIQ/artwork_cache/` keyed by SHA-256 hash of file path and file size.
- UI Binding: High-speed asynchronous image display in QML (`MiniPlayer`, `NowPlayingScreen`, `SoniqTrackRow`) with vector glyph fallback.
