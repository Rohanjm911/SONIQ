#include "AudioPlaybackEngine.hpp"
#include "output/NullAudioOutput.hpp"
#if defined(_WIN32)
#include "output/WASAPIAudioOutput.hpp"
#endif
#include "core/Logger.hpp"
#include <algorithm>
#include <chrono>

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
    m_cv.notify_all();
    if (m_audioThread.joinable()) {
        m_audioThread.join();
    }
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

core::Result<void> AudioPlaybackEngine::playTrack(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setState(core::PlaybackState::Buffering);

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
    if (m_state == core::PlaybackState::Paused && m_decoder->isOpen()) {
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
    if (m_output) m_output->stop();
    m_decoder->close();
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
    if (!m_decoder->isOpen()) {
        return core::Error::make(-1, "No track currently loaded", "Engine");
    }
    m_pendingSeek = seconds;
    m_cv.notify_one();
    return core::Result<void>::ok();
}

void AudioPlaybackEngine::nextTrack() {
    auto next = m_queue.next();
    if (next.has_value()) {
        playTrack(next->filePath);
    } else {
        stop();
    }
}

void AudioPlaybackEngine::previousTrack() {
    // If we are more than 3 seconds into the track, previous restarts track
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

    // If DSP alters signal or volume is not 100%, bit-perfect is unavailable
    if (m_dsp.altersSignal() || std::abs(m_volume.load() - 1.0f) > 1e-4f || m_muted.load()) {
        return core::BitPerfectStatus::Unavailable;
    }

    return m_output->bitPerfectStatus();
}

std::string AudioPlaybackEngine::bitPerfectStatusExplanation() const {
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

        size_t framesRead = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_decoder && m_decoder->isOpen() && !m_decoder->isEof()) {
                uint16_t ch = m_decoder->format().channels() > 0 ? m_decoder->format().channels() : 2;
                if (readBuffer.size() < kBlockFrames * ch) {
                    readBuffer.resize(kBlockFrames * ch);
                }
                framesRead = m_decoder->readFrames(readBuffer.data(), kBlockFrames);
                m_positionSeconds = m_decoder->positionSeconds();
            }
        }

        if (framesRead == 0) {
            // Track reached EOF
            SONIQ_LOG_INFO("Track finished playback, transitioning to next");
            nextTrack();
            if (m_trackEndedCallback) {
                m_trackEndedCallback();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Apply DSP
        uint16_t ch = m_decoder->format().channels() > 0 ? m_decoder->format().channels() : 2;
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
            // If output is null or paused, simulate real-time consumption
            double blockMs = (static_cast<double>(framesRead) / static_cast<double>(m_decoder->format().sampleRate())) * 1000.0;
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(blockMs));
        }
    }
}

} // namespace soniq::audio
