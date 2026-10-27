# SONIQ Supported Audio Formats Matrix

| Container / Extension | Codec Name | Lossless / Lossy | Hi-Res Support | Probing Mechanism | Decoding Engine |
|---|---|---|---|---|---|
| `.wav` | Linear PCM / IEEE Float | Lossless | Yes (up to 384kHz / 32-bit) | Direct Header Sniffing & FFprobe | Direct High-Speed PCM & FFmpeg |
| `.flac` | Free Lossless Audio Codec | Lossless | Yes (up to 192kHz / 24-bit) | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.alac`, `.m4a` | Apple Lossless Audio Codec | Lossless | Yes (up to 192kHz / 24-bit) | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.aiff`, `.aif` | Audio Interchange File Format | Lossless | Yes (up to 192kHz / 24-bit) | Direct Header Sniffing & FFprobe | Direct High-Speed PCM & FFmpeg |
| `.ape` | Monkey's Audio | Lossless | Yes | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.wv` | WavPack | Lossless / Hybrid | Yes | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.dsf`, `.dff` | Direct Stream Digital (DSD) | Lossless Bitstream | Yes (DSD64, DSD128, DSD256) | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.mp3` | MPEG-1 Audio Layer III | Lossy | No (44.1/48kHz, max 320kbps) | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.aac`, `.m4a` | Advanced Audio Coding | Lossy | No (up to 48kHz, max 320kbps) | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.ogg` | Ogg Vorbis | Lossy | No | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.opus` | Opus Interactive Audio | Lossy | Up to 48kHz | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.wma` | Windows Media Audio | Lossy / Lossless | Dependent on profile | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.ac3` | Dolby Digital AC-3 | Compressed | No | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
| `.dts` | Digital Theater Systems | Compressed | Up to 96kHz | FFprobe JSON Metadata Probe | FFmpeg Streaming Pipe |
