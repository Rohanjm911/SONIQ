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
