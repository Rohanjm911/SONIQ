#include "AudioPlaybackEngine.hpp"
#include "output/NullAudioOutput.hpp"
#if defined(_WIN32)
#include "output/WASAPIAudioOutput.hpp"
#endif
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <numbers>
#include <cmath>

namespace soniq::audio {

AudioPlaybackEngine::AudioPlaybackEngine() {
#if defined(_WIN32)
    m_output = std::make_unique<WASAPIAudioOutput>();
#else
    m_output = std::make_unique<NullAudioOutput>();
#endif
    m_decoder = std::make_unique<FFmpegAudioDecoder>();
    m_processBuffer.resize(2048, 2);

    m_audioThread = std::thread(&AudioPlaybackEngine::audioThreadLoop, this);
}

AudioPlaybackEngine::~AudioPlaybackEngine() {
    stop();
    m_stopThread = true;
    clearLookahead();
    m_cv.notify_all();
    if (m_audioThread.joinable()) {
        m_audioThread.join();
    }
}

std::unique_ptr<AudioDecoder> AudioPlaybackEngine::createDecoder(const std::string& filePath) {
    namespace fs = std::filesystem;
    std::string ext = fs::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".dsf" || ext == ".dff") {
        auto mode = m_dopEnabled ? dsd::DsdPlaybackMode::DoP : dsd::DsdPlaybackMode::DecimatedPcm;
        return std::make_unique<dsd::DsdAudioDecoder>(mode);
    }
    return std::make_unique<FFmpegAudioDecoder>();
}

void AudioPlaybackEngine::setOutputBackend(std::unique_ptr<IAudioOutput> output) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_output) {
        m_output->stop();
        m_output->close();
    }
    m_output = std::move(output);
}

void AudioPlaybackEngine::setState(core::PlaybackState newState) {
    if (m_state.exchange(newState) != newState) {
        if (m_stateCallback) {
            m_stateCallback(newState);
        }
    }
}

void AudioPlaybackEngine::clearLookahead() {
    m_lookaheadReady = false;
    m_lookaheadLoading = false;
    if (m_lookaheadWorker.joinable()) {
        m_lookaheadWorker.join();
    }
    std::lock_guard<std::mutex> lock(m_lookaheadMutex);
    m_lookaheadDecoder.reset();
    m_lookaheadTargetFile.clear();
}

void AudioPlaybackEngine::startLookaheadWorker(const std::string& filePath) {
    if (m_lookaheadLoading || m_lookaheadReady) return;
    m_lookaheadLoading = true;
    m_lookaheadTargetFile = filePath;

    if (m_lookaheadWorker.joinable()) {
        m_lookaheadWorker.join();
    }

    m_lookaheadWorker = std::thread([this, filePath]() {
        auto dec = createDecoder(filePath);
        auto res = dec->open(filePath);
        if (res.isOk()) {
            std::lock_guard<std::mutex> lock(m_lookaheadMutex);
            m_lookaheadDecoder = std::move(dec);
            m_lookaheadReady = true;
            SONIQ_LOG_INFO("Lookahead pre-buffering ready for next track: {}", filePath);
        } else {
            SONIQ_LOG_WARN("Lookahead pre-buffering failed for: {}", filePath);
        }
        m_lookaheadLoading = false;
    });
}

core::Result<void> AudioPlaybackEngine::playTrack(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setState(core::PlaybackState::Buffering);
    clearLookahead();

    m_decoder = createDecoder(filePath);
    auto openRes = m_decoder->open(filePath);
    if (openRes.isError()) {
        setState(core::PlaybackState::Error);
        return openRes;
    }

    m_durationSeconds = m_decoder->durationSeconds();
    m_positionSeconds = 0.0;
    m_pendingSeek = -1.0;

    auto format = m_decoder->format();
    m_dsp.setSampleRate(format.sampleRate());
    m_dsp.reset();

    if (m_output) {
        auto outRes = m_output->open(format, "", core::OutputMode::Shared);
        if (outRes.isOk()) {
            m_output->start();
        }
    }

    setState(core::PlaybackState::Playing);
    m_cv.notify_one();
    return core::Result<void>::ok();
}

void AudioPlaybackEngine::play() {
    if (m_state == core::PlaybackState::Paused && m_decoder && m_decoder->isOpen()) {
        if (m_output) m_output->start();
        setState(core::PlaybackState::Playing);
        m_cv.notify_one();
    } else if (m_state == core::PlaybackState::Stopped) {
        auto cur = m_queue.currentItem();
        if (cur.has_value()) {
            playTrack(cur->filePath);
        }
    }
}

void AudioPlaybackEngine::pause() {
    if (m_state == core::PlaybackState::Playing) {
        if (m_output) m_output->pause();
        setState(core::PlaybackState::Paused);
    }
}

void AudioPlaybackEngine::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    clearLookahead();
    if (m_output) m_output->stop();
    if (m_decoder) m_decoder->close();
    m_positionSeconds = 0.0;
    setState(core::PlaybackState::Stopped);
}

void AudioPlaybackEngine::togglePlayPause() {
    if (m_state == core::PlaybackState::Playing) {
        pause();
    } else {
        play();
    }
}

core::Result<void> AudioPlaybackEngine::seek(double seconds) {
    if (!m_decoder || !m_decoder->isOpen()) {
        return core::Error::make(-1, "No track currently loaded", "Engine");
    }
    clearLookahead();
    m_pendingSeek = seconds;
    m_cv.notify_one();
    return core::Result<void>::ok();
}

void AudioPlaybackEngine::nextTrack() {
    clearLookahead();
    auto next = m_queue.next();
    if (next.has_value()) {
        playTrack(next->filePath);
    } else {
        stop();
    }
}

void AudioPlaybackEngine::previousTrack() {
    clearLookahead();
    if (m_positionSeconds.load() > 3.0) {
        seek(0.0);
        return;
    }

    auto prev = m_queue.previous();
    if (prev.has_value()) {
        playTrack(prev->filePath);
    } else {
        seek(0.0);
    }
}

void AudioPlaybackEngine::setVolume(float volumeLinear) {
    m_volume = std::clamp(volumeLinear, 0.0f, 1.0f);
    if (m_output) {
        m_output->setVolume(m_volume);
    }
}

void AudioPlaybackEngine::setMuted(bool muted) {
    m_muted = muted;
    if (m_output) {
        m_output->setMuted(muted);
    }
}

core::AudioFormat AudioPlaybackEngine::currentFormat() const noexcept {
    if (m_decoder && m_decoder->isOpen()) {
        return m_decoder->format();
    }
    return core::AudioFormat();
}

core::BitPerfectStatus AudioPlaybackEngine::bitPerfectStatus() const noexcept {
    if (!m_output) return core::BitPerfectStatus::Unknown;

    // Crossfading and DSP alter signal
    if (m_crossfadeEnabled.load() || m_dsp.altersSignal() || std::abs(m_volume.load() - 1.0f) > 1e-4f || m_muted.load()) {
        return core::BitPerfectStatus::Unavailable;
    }

    return m_output->bitPerfectStatus();
}

std::string AudioPlaybackEngine::bitPerfectStatusExplanation() const {
    if (m_crossfadeEnabled.load()) {
        return "Equal-power crossfading active (track overlap mixing alters original sample bits).";
    }

    if (m_dsp.altersSignal()) {
        auto reasons = m_dsp.bitPerfectDisablingReasons();
        std::string explanation = "Bit-perfect mode is disabled by DSP processing:\n";
        for (const auto& r : reasons) {
            explanation += " • " + r + "\n";
        }
        return explanation;
    }

    if (std::abs(m_volume.load() - 1.0f) > 1e-4f) {
        return "Software volume is attenuated. Set volume to 100% to preserve original bitstream values.";
    }

    if (m_muted.load()) {
        return "Audio output is muted.";
    }

    if (m_output) {
        return m_output->bitPerfectStatusExplanation();
    }

    return "No audio output device active.";
}

void AudioPlaybackEngine::setVisualizerCallback(VisualizerCallback cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_visualizerCallback = std::move(cb);
}

void AudioPlaybackEngine::setStateChangedCallback(StateChangedCallback cb) {
    m_stateCallback = std::move(cb);
}

void AudioPlaybackEngine::setTrackEndedCallback(TrackEndedCallback cb) {
    m_trackEndedCallback = std::move(cb);
}

void AudioPlaybackEngine::audioThreadLoop() {
    constexpr size_t kBlockFrames = 1024;
    std::vector<float> readBuffer(kBlockFrames * 2, 0.0f);
    std::vector<float> crossfadeBuffer(kBlockFrames * 2, 0.0f);

    while (!m_stopThread) {
        if (m_state != core::PlaybackState::Playing) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait_for(lock, std::chrono::milliseconds(20), [this] {
                return m_stopThread || m_state == core::PlaybackState::Playing || m_pendingSeek.load() >= 0.0;
            });
            if (m_stopThread) break;
        }

        // Handle seek if pending
        double seekSec = m_pendingSeek.exchange(-1.0);
        if (seekSec >= 0.0) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_decoder && m_decoder->isOpen()) {
                m_decoder->seek(seekSec);
                m_positionSeconds = m_decoder->positionSeconds();
            }
        }

        if (m_state != core::PlaybackState::Playing) {
            continue;
        }

        // Lookahead trigger: if track has <= 4.0 seconds remaining, prime next track
        double curPos = m_positionSeconds.load();
        double totalDur = m_durationSeconds.load();
        double remainingSec = totalDur - curPos;

        if ((m_gaplessEnabled.load() || m_crossfadeEnabled.load()) &&
            remainingSec <= std::max(4.0, m_crossfadeSeconds.load() + 1.0) &&
            !m_lookaheadReady && !m_lookaheadLoading && m_queue.hasNext()) {

            auto nextItem = m_queue.peekNext();
            if (nextItem.has_value()) {
                startLookaheadWorker(nextItem->filePath);
            }
        }

        size_t framesRead = 0;
        uint16_t ch = 2;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_decoder && m_decoder->isOpen() && !m_decoder->isEof()) {
                ch = m_decoder->format().channels() > 0 ? m_decoder->format().channels() : 2;
                if (readBuffer.size() < kBlockFrames * ch) {
                    readBuffer.resize(kBlockFrames * ch);
                }
                framesRead = m_decoder->readFrames(readBuffer.data(), kBlockFrames);
                m_positionSeconds = m_decoder->positionSeconds();
            }
        }

        // Equal-Power Crossfading logic
        if (m_crossfadeEnabled.load() && m_lookaheadReady.load() && remainingSec <= m_crossfadeSeconds.load() && remainingSec > 0.0 && framesRead > 0) {
            std::lock_guard<std::mutex> lock(m_lookaheadMutex);
            if (m_lookaheadDecoder && m_lookaheadDecoder->isOpen() && !m_lookaheadDecoder->isEof()) {
                if (crossfadeBuffer.size() < framesRead * ch) {
                    crossfadeBuffer.resize(framesRead * ch);
                }
                size_t nextRead = m_lookaheadDecoder->readFrames(crossfadeBuffer.data(), framesRead);

                double xfDuration = m_crossfadeSeconds.load();
                double progress = std::clamp(1.0 - (remainingSec / xfDuration), 0.0, 1.0);

                // Equal power curves: cos(pi/2 * p) and sin(pi/2 * p)
                float gainOut = static_cast<float>(std::cos(0.5 * std::numbers::pi * progress));
                float gainIn = static_cast<float>(std::sin(0.5 * std::numbers::pi * progress));

                for (size_t i = 0; i < framesRead * ch; ++i) {
                    float inSample = (i < nextRead * ch) ? crossfadeBuffer[i] : 0.0f;
                    readBuffer[i] = (readBuffer[i] * gainOut) + (inSample * gainIn);
                }
            }
        }

        // Track EOF or transition
        if (framesRead == 0) {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::lock_guard<std::mutex> lookaheadLock(m_lookaheadMutex);

            if (m_lookaheadReady && m_lookaheadDecoder && m_lookaheadDecoder->isOpen()) {
                bool matchingFormat = (m_decoder &&
                                       m_decoder->format().sampleRate() == m_lookaheadDecoder->format().sampleRate() &&
                                       m_decoder->format().channels() == m_lookaheadDecoder->format().channels());

                m_decoder = std::move(m_lookaheadDecoder);
                m_queue.advanceNext();
                m_durationSeconds = m_decoder->durationSeconds();
                m_positionSeconds = m_decoder->positionSeconds();
                m_lookaheadReady = false;
                m_lookaheadLoading = false;

                if (matchingFormat) {
                    // Seamless gapless: keep WASAPI stream running without restart
                    SONIQ_LOG_INFO("Seamless gapless transition executed (output format preserved)");
                } else {
                    // Format changed: reconfigure output
                    auto format = m_decoder->format();
                    m_dsp.setSampleRate(format.sampleRate());
                    if (m_output) {
                        m_output->open(format, "", core::OutputMode::Shared);
                        m_output->start();
                    }
                    SONIQ_LOG_INFO("Gapless transition with format adaptation: {} Hz", format.sampleRate());
                }

                if (m_trackEndedCallback) {
                    m_trackEndedCallback();
                }
                continue;
            } else {
                SONIQ_LOG_INFO("Track finished playback, transitioning to next");
                nextTrack();
                if (m_trackEndedCallback) {
                    m_trackEndedCallback();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        }

        // Apply DSP
        m_dsp.process(readBuffer.data(), framesRead, ch);

        // Visualizer data extraction
        if (m_visualizerCallback) {
            EngineVisualizerData viz;
            viz.waveform.assign(readBuffer.begin(), readBuffer.begin() + std::min(readBuffer.size(), size_t(128)));
            viz.peakLevelL = 0.0f;
            viz.peakLevelR = 0.0f;
            for (size_t f = 0; f < framesRead; ++f) {
                float l = std::abs(readBuffer[f * ch + 0]);
                float r = (ch > 1) ? std::abs(readBuffer[f * ch + 1]) : l;
                if (l > viz.peakLevelL) viz.peakLevelL = l;
                if (r > viz.peakLevelR) viz.peakLevelR = r;
            }
            m_visualizerCallback(viz);
        }

        // Write to audio output
        if (m_output && m_output->isRunning()) {
            size_t written = 0;
            while (written < framesRead && m_state == core::PlaybackState::Playing && !m_stopThread) {
                size_t n = m_output->write(readBuffer.data() + (written * ch), framesRead - written);
                written += n;
                if (n == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
            }
        } else {
            // Null output real-time simulation
            double sr = (m_decoder && m_decoder->format().sampleRate() > 0) ? m_decoder->format().sampleRate() : 44100.0;
            double blockMs = (static_cast<double>(framesRead) / sr) * 1000.0;
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(blockMs));
        }
    }
}

} // namespace soniq::audio
