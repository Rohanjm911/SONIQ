#pragma once

#include "AudioDecoder.hpp"
#include "FFmpegAudioDecoder.hpp"
#include "dsd/DsdAudioDecoder.hpp"
#include "AudioBuffer.hpp"
#include "PlaybackQueue.hpp"
#include "dsp/DspPipeline.hpp"
#include "output/IAudioOutput.hpp"
#include "core/Types.hpp"
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

namespace soniq::audio {

struct EngineVisualizerData {
    std::vector<float> waveform;
    float peakLevelL{0.0f};
    float peakLevelR{0.0f};
};

class AudioPlaybackEngine {
public:
    using VisualizerCallback = std::function<void(const EngineVisualizerData&)>;
    using StateChangedCallback = std::function<void(core::PlaybackState newState)>;
    using TrackEndedCallback = std::function<void()>;

    AudioPlaybackEngine();
    ~AudioPlaybackEngine();

    void setOutputBackend(std::unique_ptr<IAudioOutput> output);
    [[nodiscard]] IAudioOutput* outputBackend() noexcept { return m_output.get(); }

    [[nodiscard]] dsp::DspPipeline& dsp() noexcept { return m_dsp; }
    [[nodiscard]] PlaybackQueue& queue() noexcept { return m_queue; }

    // Playback control
    core::Result<void> playTrack(const std::string& filePath);
    void play();
    void pause();
    void stop();
    void togglePlayPause();

    core::Result<void> seek(double seconds);
    void nextTrack();
    void previousTrack();

    void setVolume(float volumeLinear);
    [[nodiscard]] float volume() const noexcept { return m_volume; }

    void setMuted(bool muted);
    [[nodiscard]] bool isMuted() const noexcept { return m_muted; }

    // Gapless, Crossfade, and DSD configuration
    void setGaplessEnabled(bool enabled) noexcept { m_gaplessEnabled = enabled; }
    [[nodiscard]] bool isGaplessEnabled() const noexcept { return m_gaplessEnabled; }

    void setCrossfade(bool enabled, double seconds) noexcept {
        m_crossfadeEnabled = enabled;
        m_crossfadeSeconds = std::max(0.1, seconds);
    }
    [[nodiscard]] bool isCrossfadeEnabled() const noexcept { return m_crossfadeEnabled; }
    [[nodiscard]] double crossfadeDuration() const noexcept { return m_crossfadeSeconds; }

    void setDopEnabled(bool enabled) noexcept { m_dopEnabled = enabled; }
    [[nodiscard]] bool isDopEnabled() const noexcept { return m_dopEnabled; }

    // Status queries
    [[nodiscard]] core::PlaybackState state() const noexcept { return m_state; }
    [[nodiscard]] double currentPositionSeconds() const noexcept { return m_positionSeconds; }
    [[nodiscard]] double totalDurationSeconds() const noexcept { return m_durationSeconds; }
    [[nodiscard]] core::AudioFormat currentFormat() const noexcept;
    [[nodiscard]] core::BitPerfectStatus bitPerfectStatus() const noexcept;
    [[nodiscard]] std::string bitPerfectStatusExplanation() const;

    // Callbacks
    void setVisualizerCallback(VisualizerCallback cb);
    void setStateChangedCallback(StateChangedCallback cb);
    void setTrackEndedCallback(TrackEndedCallback cb);

private:
    void audioThreadLoop();
    void setState(core::PlaybackState newState);
    std::unique_ptr<AudioDecoder> createDecoder(const std::string& filePath);
    void startLookaheadWorker(const std::string& filePath);
    void clearLookahead();

    std::unique_ptr<IAudioOutput> m_output;
    std::unique_ptr<AudioDecoder> m_decoder;
    std::unique_ptr<AudioDecoder> m_lookaheadDecoder;

    dsp::DspPipeline m_dsp;
    PlaybackQueue m_queue;

    std::atomic<core::PlaybackState> m_state{core::PlaybackState::Stopped};
    std::atomic<bool> m_stopThread{false};
    std::atomic<float> m_volume{1.0f};
    std::atomic<bool> m_muted{false};
    std::atomic<double> m_positionSeconds{0.0};
    std::atomic<double> m_durationSeconds{0.0};
    std::atomic<double> m_pendingSeek{-1.0};

    // Gapless & Crossfade controls
    std::atomic<bool> m_gaplessEnabled{true};
    std::atomic<bool> m_crossfadeEnabled{false};
    std::atomic<double> m_crossfadeSeconds{2.0};
    std::atomic<bool> m_dopEnabled{true};

    std::atomic<bool> m_lookaheadReady{false};
    std::atomic<bool> m_lookaheadLoading{false};
    std::string m_lookaheadTargetFile;
    std::thread m_lookaheadWorker;
    mutable std::recursive_mutex m_lookaheadMutex;

    std::thread m_audioThread;
    mutable std::recursive_mutex m_mutex;
    std::condition_variable_any m_cv;

    VisualizerCallback m_visualizerCallback;
    StateChangedCallback m_stateCallback;
    TrackEndedCallback m_trackEndedCallback;

    AudioBuffer m_processBuffer;
};

} // namespace soniq::audio
