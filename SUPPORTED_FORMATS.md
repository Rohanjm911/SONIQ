# SONIQ — Supported Audio Formats & Codec Architecture

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/b/bb/Hi-Res_Audio_logo.svg" width="90" alt="Hi-Res Audio Official Logo" />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://upload.wikimedia.org/wikipedia/commons/7/7b/Direct_Stream_Digital_logo.svg" width="130" alt="DSD Official Logo" />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://upload.wikimedia.org/wikipedia/commons/7/79/Flac_logo.svg" width="110" alt="FLAC Official Logo" />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://upload.wikimedia.org/wikipedia/commons/0/02/Opus_logo.svg" width="110" alt="Opus Official Logo" />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://upload.wikimedia.org/wikipedia/commons/7/7a/Dolby_Digital_logo.svg" width="100" alt="Dolby Digital Logo" />
</p>

> **SONIQ** provides universal format compatibility, delivering bit-perfect hardware passthrough for studio masters and audiophile bitstreams, alongside low-latency software decoders for everyday lossy containers.

---

## 1. Comprehensive Formats & Codecs Matrix

| Format | Official Logo | Family | Max Sample Rate | Max Bit Depth | Channels | Probing Engine | Decoding Pipeline | Bit-Perfect |
| :--- | :---: | :--- | :--- | :--- | :--- | :--- | :--- | :---: |
| **DSD / SACD**<br>`.dsf`, `.dff` | <img src="https://upload.wikimedia.org/wikipedia/commons/7/7b/Direct_Stream_Digital_logo.svg" width="60" alt="DSD" /> | 1-Bit Delta-Sigma | 11.2896 MHz (DSD256) | 1-bit bitstream | 2.0, 5.1 | Direct DSF/DFF Chunk Parser | DoP v1.1 Unpacker / 64-Tap FIR Decimator | **✓ Yes** |
| **FLAC**<br>`.flac` | <img src="https://upload.wikimedia.org/wikipedia/commons/7/79/Flac_logo.svg" width="55" alt="FLAC" /> | Lossless Compression | 192 kHz (up to 384 kHz) | 16, 24, 32-bit | Up to 8 ch | Native Picture Block & Streaminfo Probe | Async 32-bit Float Pipeline | **✓ Yes** |
| **WAV / Waveform**<br>`.wav`, `.wave` | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="35" alt="WAV" /> | Uncompressed PCM | 768 kHz | 8, 16, 24, 32-bit Int/Float | Up to 8 ch | Direct RIFF/WAVE Header Sniffer | Direct Zero-Latency PCM Reader | **✓ Yes** |
| **ALAC (Apple Lossless)**<br>`.m4a`, `.alac` | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/apple/apple-original.svg" width="35" alt="Apple" /> | Lossless Compression | 192 kHz | 16, 24-bit | Stereo, 5.1 | QuickTime Atom / MP4 moov Probe | High-Throughput Stream Pipeline | **✓ Yes** |
| **AIFF**<br>`.aiff`, `.aif` | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/apple/apple-original.svg" width="35" alt="AIFF" /> | Uncompressed PCM | 192 kHz | 16, 24, 32-bit | Stereo, Multi | Direct IFF/AIFF Header Sniffer | Direct Memory-Mapped PCM Reader | **✓ Yes** |
| **WavPack**<br>`.wv` | <img src="https://upload.wikimedia.org/wikipedia/commons/b/bb/Hi-Res_Audio_logo.svg" width="40" alt="Hi-Res" /> | Lossless / Hybrid | 192 kHz | 16, 24, 32-bit Float | Up to 8 ch | Stream Header Probe | Universal Stream Pipeline | **✓ Yes** |
| **Monkey's Audio**<br>`.ape` | <img src="https://upload.wikimedia.org/wikipedia/commons/b/bb/Hi-Res_Audio_logo.svg" width="40" alt="Hi-Res" /> | Lossless Compression | 192 kHz | 16, 24-bit | Stereo | APE Header & Tag Deserializer | Universal Stream Pipeline | **✓ Yes** |
| **Opus**<br>`.opus` | <img src="https://upload.wikimedia.org/wikipedia/commons/0/02/Opus_logo.svg" width="55" alt="Opus" /> | Lossy Interactive | 48 kHz | 32-bit Float Internal | Stereo, 5.1 | Ogg OpusHead Probe | Floating-Point Resampled Stream | Shared Mode |
| **Ogg Vorbis**<br>`.ogg` | <img src="https://upload.wikimedia.org/wikipedia/commons/1/1b/Ogg_Logo.svg" width="45" alt="Ogg" /> | Lossy Variable Bitrate | 48 kHz (up to 96 kHz) | 16-bit Equivalent | Stereo, 5.1 | Ogg Page & Vorbis Comment Probe | Floating-Point Stream Pipeline | Shared Mode |
| **Advanced Audio Coding (AAC)**<br>`.aac`, `.m4a` | <img src="https://upload.wikimedia.org/wikipedia/commons/7/7a/Dolby_Digital_logo.svg" width="50" alt="Dolby" /> | Lossy Perceptual | 48 kHz (up to 96 kHz) | 16-bit Equivalent | Stereo, 5.1, 7.1 | ADTS / MP4 ES Descriptor Probe | Universal Stream Pipeline | Shared Mode |
| **MP3**<br>`.mp3` | <img src="https://upload.wikimedia.org/wikipedia/commons/d/df/Xiph.org_logo.svg" width="40" alt="MP3" /> | Lossy MPEG-1 Layer 3 | 44.1 / 48 kHz | 16-bit Equivalent | Stereo, Joint | ID3v2 Synchsafe Tag Probe | High-Precision Layer III Stream | Shared Mode |
| **Dolby Digital**<br>`.ac3` | <img src="https://upload.wikimedia.org/wikipedia/commons/7/7a/Dolby_Digital_logo.svg" width="55" alt="Dolby" /> | Multichannel Compressed | 48 kHz | 16-bit Equivalent | Up to 5.1 | Syncword `0x0B77` Bitstream Probe | Multichannel Downmix / Passthrough | Shared Mode |
| **Digital Theater Systems (DTS)**<br>`.dts` | <img src="https://upload.wikimedia.org/wikipedia/commons/2/29/DTS_logo.svg" width="55" alt="DTS" /> | Multichannel Lossy/Hi-Res | 96 kHz | Up to 24-bit | Up to 5.1 / 7.1 | Core Stream Frame Probe | Multichannel Coaxial / Downmix | Shared Mode |
| **Windows Media Audio**<br>`.wma` | <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="35" alt="WMA" /> | Lossy / Lossless Profile | 48 kHz / 96 kHz | 16, 24-bit | Stereo, 5.1 | ASF Header Object Probe | Universal Stream Pipeline | Profile-Dep. |

---

## 2. In-Depth Format Architecture & Characteristics

### <img src="https://upload.wikimedia.org/wikipedia/commons/7/7b/Direct_Stream_Digital_logo.svg" width="70" align="absmiddle" alt="DSD" /> Direct Stream Digital (DSD / SACD)
* **Extensions:** `.dsf`, `.dff`
* **Bit Depth:** 1-Bit Direct Delta-Sigma modulated pulse-density stream.
* **Sampling Rates:**
  * **DSD64**: $2.8224\text{ MHz}$ ($64 \times 44.1\text{ kHz}$ — Super Audio CD physical media standard).
  * **DSD128**: $5.6448\text{ MHz}$ ($128 \times 44.1\text{ kHz}$ — Double-rate DSD).
  * **DSD256**: $11.2896\text{ MHz}$ ($256 \times 44.1\text{ kHz}$ — Quad-rate DSD studio master).
* **Dual Execution Paths:**
  1. **Hardware Bit-Perfect DoP v1.1 (DSD over PCM):**
     * Formats 1-bit DSD samples into standard 24-bit PCM packages (8-bit alternating markers `0x05`/`0xFA` + 16 bits of raw DSD data) at $176.4\text{ kHz}$ or $352.8\text{ kHz}$.
     * Your external DAC detects the synchronization signatures and switches its internal delta-sigma filter directly to native DSD mode.
  2. **High-Precision 64-Tap Linear Phase FIR Decimation Filter:**
     * For standard PCM DACs and sound cards, converts 1-bit DSD to 32-bit floating point PCM using an optimized FIR decimation filter with a steep $40\text{ kHz}$ cutoff, preventing high-frequency ultrasonic modulation noise.

---

### <img src="https://upload.wikimedia.org/wikipedia/commons/7/79/Flac_logo.svg" width="60" align="absmiddle" alt="FLAC" /> Free Lossless Audio Codec (FLAC)
* **Extensions:** `.flac`
* **Characteristics:** Open-source, bit-exact compression algorithm reducing raw audio file sizes by 40–60% without dropping a single bit of audio data.
* **Mastering Resolution:** Supports 16-bit (Red Book CD), 24-bit studio masters, and up to 32-bit integer containers up to $192\text{ kHz}$ (and experimental $384\text{ kHz}$).
* **Metadata Architecture:** Fully supports Vorbis Comments, embedded CUE sheets, and high-resolution album covers inside binary `METADATA_BLOCK_PICTURE` (Front Cover Type 3 / 6).

---

### <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="30" align="absmiddle" alt="WAV" /> Waveform Audio File Format (WAV / RIFF)
* **Extensions:** `.wav`, `.wave`
* **Characteristics:** Uncompressed pulse-code modulation (PCM) container developed by Microsoft and IBM.
* **Zero-Latency Decoding:** SONIQ employs a direct binary header reader for WAV files, bypassing complex container decoders for instant seek response and zero pre-buffer delay.
* **Compatibility:** Integer PCM (8, 16, 24, 32-bit) and IEEE 754 Floating-Point (32-bit float, 64-bit double).

---

### <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/apple/apple-original.svg" width="30" align="absmiddle" alt="Apple" /> Apple Lossless Audio Codec (ALAC)
* **Extensions:** `.m4a`, `.alac`
* **Characteristics:** Proprietary lossless compression created by Apple, later open-sourced in 2011.
* **Resolution:** Up to 24-bit / $192\text{ kHz}$.
* **Compatibility:** Full native reading of MPEG-4 ISO/IEC 14496-14 atom trees (`moov`, `trak`, `mdia`, `minf`, `stbl`, `stsd`).

---

### <img src="https://upload.wikimedia.org/wikipedia/commons/0/02/Opus_logo.svg" width="60" align="absmiddle" alt="Opus" /> Opus Interactive Audio Codec
* **Extensions:** `.opus`
* **Characteristics:** Standardized by IETF (RFC 6716), combining technology from Skype's SILK and Xiph.Org's CELT.
* **Acoustic Performance:** Surpasses MP3, AAC, and Vorbis at comparable bitrates; seamlessly handles speech and complex orchestral music with near-lossless transparent quality at $160\text{--}320\text{ kbps}$.

---

### <img src="https://upload.wikimedia.org/wikipedia/commons/7/7a/Dolby_Digital_logo.svg" width="60" align="absmiddle" alt="Dolby" /> & <img src="https://upload.wikimedia.org/wikipedia/commons/2/29/DTS_logo.svg" width="60" align="absmiddle" alt="DTS" /> Cinema Multichannel (AC-3 & DTS)
* **Extensions:** `.ac3`, `.dts`
* **Surround Capability:** 5.1 and 7.1 discrete channels (Front Left, Center, Front Right, Surround Left, Surround Right, LFE Subwoofer).
* **Downmixing Engine:** Intelligently downmixes multichannel audio into stereo using ITU-R BS.775 coefficient matrices when playing through stereo speakers or headphones.

---

## 3. Metadata & Tagging Standards Support

SONIQ includes a unified metadata extractor and built-in editor that reads and writes all industry-standard tagging formats:

| Tag Standard | Supported Containers | Embedded Artwork Support | Read | Write (Tag Editor) |
| :--- | :--- | :---: | :---: | :---: |
| **ID3v2.3 / ID3v2.4** | `.mp3`, `.wav`, `.aiff`, `.dsf` | **Yes** (APIC frame, JPEG/PNG) | **✓ Yes** | **✓ Yes** |
| **ID3v1 / ID3v1.1** | `.mp3` | No | **✓ Yes** | Legacy Only |
| **Vorbis Comments** | `.flac`, `.ogg`, `.opus` | **Yes** (`METADATA_BLOCK_PICTURE`) | **✓ Yes** | **✓ Yes** |
| **MP4 / iTunes Atoms** | `.m4a`, `.alac`, `.aac` | **Yes** (`covr` box) | **✓ Yes** | **✓ Yes** |
| **APEv2 Tags** | `.ape`, `.wv`, `.mpc` | **Yes** (Binary cover items) | **✓ Yes** | **✓ Yes** |
| **RIFF INFO List** | `.wav` | External file (`cover.jpg`) | **✓ Yes** | **✓ Yes** |

---

## 4. Decoding Pipeline & Signal Purity Architecture

```
                    ┌───────────────────────────────┐
                    │     Input Audio File (.ext)   │
                    └──────────────┬────────────────┘
                                   │
                    Format Inspection & Probing
                                   │
              ┌────────────────────┼────────────────────┐
              ▼                    ▼                    ▼
     Direct RIFF Parser      DSD/DoP Engine       Universal Pipeline
     (Zero-Latency WAV)      (DSF / DFF SACD)     (FLAC/ALAC/Opus/MP3)
              │                    │                    │
              ▼                    ▼                    ▼
     32-Bit Float PCM       DoP 16-Bit Frame      32-Bit Float PCM
              │             or 64-Tap FIR               │
              └────────────────────┬────────────────────┘
                                   │
                      Is Audio Modification Active?
                       (EQ / Volume / Crossfeed)
                                   │
                     ┌─────────────┴─────────────┐
                     ▼ NO                        ▼ YES
        ┌─────────────────────────┐   ┌─────────────────────────┐
        │  Bit-Perfect Streaming  │   │   64-Bit Studio DSP     │
        │  Unaltered Raw Audio    │   │  Biquad EQ, BS2B, Limit │
        └────────────┬────────────┘   └────────────┬────────────┘
                     │                             │
                     └─────────────┬───────────────┘
                                   ▼
                    ┌─────────────────────────────┐
                    │   Windows WASAPI Exclusive   │
                    │   Kernel Audio Endpoint     │
                    └─────────────────────────────┘
```

1. **Direct Memory Mapping & Header Sniffing:**
   - PCM files (`.wav`, `.aiff`) bypass subprocess pipelines for instant microsecond playback initiation.
2. **Subprocess Isolation:**
   - FFmpeg and FFprobe run asynchronously using native Win32 `CreateProcessW` with `CREATE_NO_WINDOW`, preventing desktop command prompt flashes while decoding edge-case audio containers.
3. **Audit Verification:**
   - In WASAPI Exclusive Mode with Volume at 100% and DSP disabled, samples flow from disk to DAC in bit-exact parity, earning the **VERIFIED BIT-PERFECT** status badge in the UI.
