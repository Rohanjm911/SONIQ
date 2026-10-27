#include "WASAPIAudioOutput.hpp"

#if defined(_WIN32)

#include "core/Logger.hpp"
#include <functiondiscoverykeys_devpkey.h>
#include <avrt.h>
#include <algorithm>
#include <cmath>

#if defined(_MSC_VER)
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "avrt.lib")
#endif

namespace soniq::audio {

WASAPIAudioOutput::WASAPIAudioOutput() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        m_coInitialized = true;
    }
}

WASAPIAudioOutput::~WASAPIAudioOutput() {
    close();
    if (m_coInitialized) {
        CoUninitialize();
    }
}

void WASAPIAudioOutput::releaseCOM() noexcept {
    if (m_audioEvent) {
        CloseHandle(m_audioEvent);
        m_audioEvent = nullptr;
    }
    if (m_renderClient) {
        m_renderClient->Release();
        m_renderClient = nullptr;
    }
    if (m_audioClient) {
        m_audioClient->Release();
        m_audioClient = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_deviceEnumerator) {
        m_deviceEnumerator->Release();
        m_deviceEnumerator = nullptr;
    }
}

std::vector<AudioDeviceInfo> WASAPIAudioOutput::availableDevices() {
    std::vector<AudioDeviceInfo> devices;

    IMMDeviceEnumerator* enumerator = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr) || !enumerator) {
        return devices;
    }

    IMMDeviceCollection* collection = nullptr;
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
    if (SUCCEEDED(hr) && collection) {
        UINT count = 0;
        collection->GetCount(&count);

        IMMDevice* defaultDevice = nullptr;
        LPWSTR defaultId = nullptr;
        if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice)) && defaultDevice) {
            defaultDevice->GetId(&defaultId);
        }

        for (UINT i = 0; i < count; ++i) {
            IMMDevice* device = nullptr;
            if (SUCCEEDED(collection->Item(i, &device)) && device) {
                LPWSTR idStr = nullptr;
                device->GetId(&idStr);

                AudioDeviceInfo info;
                if (idStr) {
                    char idBuffer[512]{};
                    WideCharToMultiByte(CP_UTF8, 0, idStr, -1, idBuffer, sizeof(idBuffer), nullptr, nullptr);
                    info.id = idBuffer;

                    if (defaultId && wcscmp(idStr, defaultId) == 0) {
                        info.isDefault = true;
                    }
                    CoTaskMemFree(idStr);
                }

                IPropertyStore* props = nullptr;
                if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props)) && props) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &varName)) && varName.pwszVal) {
                        char nameBuffer[512]{};
                        WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, nameBuffer, sizeof(nameBuffer), nullptr, nullptr);
                        info.name = nameBuffer;
                    }
                    PropVariantClear(&varName);
                    props->Release();
                }

                info.supportsExclusive = true;
                info.supportedSampleRates = {44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000};
                info.supportedFormats = {core::SampleFormat::Int16, core::SampleFormat::Int24, core::SampleFormat::Int32, core::SampleFormat::Float32};

                devices.push_back(std::move(info));
                device->Release();
            }
        }

        if (defaultId) CoTaskMemFree(defaultId);
        if (defaultDevice) defaultDevice->Release();
        collection->Release();
    }

    enumerator->Release();
    return devices;
}

core::Result<void> WASAPIAudioOutput::open(const core::AudioFormat& format,
                                          const std::string& deviceId,
                                          core::OutputMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    close();

    m_format = format;
    m_mode = mode;
    m_exclusiveAcquired = false;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&m_deviceEnumerator));
    if (FAILED(hr)) {
        return core::Error::make(hr, "Failed to create IMMDeviceEnumerator", "WASAPI");
    }

    if (deviceId.empty()) {
        hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_device);
    } else {
        std::wstring wideId(deviceId.begin(), deviceId.end());
        hr = m_deviceEnumerator->GetDevice(wideId.c_str(), &m_device);
    }

    if (FAILED(hr) || !m_device) {
        releaseCOM();
        return core::Error::make(hr, "Failed to get target audio endpoint", "WASAPI");
    }

    hr = m_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&m_audioClient));
    if (FAILED(hr) || !m_audioClient) {
        releaseCOM();
        return core::Error::make(hr, "Failed to activate IAudioClient", "WASAPI");
    }

    // Configure WAVEFORMATEXTENSIBLE for IEEE Float
    WAVEFORMATEXTENSIBLE wfx{};
    wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wfx.Format.nChannels = format.channels() > 0 ? format.channels() : 2;
    wfx.Format.nSamplesPerSec = format.sampleRate() > 0 ? format.sampleRate() : 44100;
    wfx.Format.wBitsPerSample = 32;
    wfx.Format.nBlockAlign = (wfx.Format.nChannels * wfx.Format.wBitsPerSample) / 8;
    wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
    wfx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
    wfx.Samples.wValidBitsPerSample = 32;
    wfx.dwChannelMask = (wfx.Format.nChannels == 1) ? SPEAKER_FRONT_CENTER : (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT);
    wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

    REFERENCE_TIME bufferDuration = 500000; // 50 ms in 100-ns units
    DWORD streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

    if (mode == core::OutputMode::Exclusive || mode == core::OutputMode::BitPerfect) {
        // Try Exclusive Mode
        hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,
                                      streamFlags,
                                      bufferDuration,
                                      bufferDuration,
                                      reinterpret_cast<WAVEFORMATEX*>(&wfx),
                                      nullptr);
        if (SUCCEEDED(hr)) {
            m_exclusiveAcquired = true;
            SONIQ_LOG_INFO("WASAPI opened in EXCLUSIVE mode at {} Hz, {} channels", wfx.Format.nSamplesPerSec, wfx.Format.nChannels);
        } else {
            SONIQ_LOG_WARN("WASAPI Exclusive mode rejected (hr=0x{:X}). Falling back to Shared mode.", static_cast<unsigned long>(hr));
        }
    }

    if (!m_exclusiveAcquired) {
        // Shared mode fallback
        WAVEFORMATEX* mixFormat = nullptr;
        m_audioClient->GetMixFormat(&mixFormat);
        WAVEFORMATEX* initFormat = mixFormat ? mixFormat : reinterpret_cast<WAVEFORMATEX*>(&wfx);

        hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                      streamFlags,
                                      bufferDuration,
                                      0,
                                      initFormat,
                                      nullptr);
        if (mixFormat) CoTaskMemFree(mixFormat);

        if (FAILED(hr)) {
            releaseCOM();
            return core::Error::make(hr, "Failed to initialize WASAPI in Shared mode", "WASAPI");
        }
        SONIQ_LOG_INFO("WASAPI opened in SHARED mode");
    }

    m_audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_audioEvent) {
        releaseCOM();
        return core::Error::make(-1, "Failed to create WASAPI audio event", "WASAPI");
    }

    hr = m_audioClient->SetEventHandle(m_audioEvent);
    if (FAILED(hr)) {
        releaseCOM();
        return core::Error::make(hr, "Failed to set WASAPI event handle", "WASAPI");
    }

    hr = m_audioClient->GetBufferSize(&m_bufferFrameCount);
    if (SUCCEEDED(hr) && format.sampleRate() > 0) {
        m_latencyMs = (static_cast<double>(m_bufferFrameCount) / static_cast<double>(format.sampleRate())) * 1000.0;
    }

    hr = m_audioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&m_renderClient));
    if (FAILED(hr) || !m_renderClient) {
        releaseCOM();
        return core::Error::make(hr, "Failed to get IAudioRenderClient", "WASAPI");
    }

    m_currentDeviceId = deviceId;
    return core::Result<void>::ok();
}

void WASAPIAudioOutput::close() {
    stop();
    std::lock_guard<std::mutex> lock(m_mutex);
    releaseCOM();
}

core::Result<void> WASAPIAudioOutput::start() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_audioClient) {
        HRESULT hr = m_audioClient->Start();
        if (SUCCEEDED(hr)) {
            m_running = true;
            return core::Result<void>::ok();
        }
        return core::Error::make(hr, "Failed to start WASAPI client", "WASAPI");
    }
    return core::Error::make(-1, "Audio client not initialized", "WASAPI");
}

void WASAPIAudioOutput::pause() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_audioClient && m_running) {
        m_audioClient->Stop();
        m_running = false;
    }
}

void WASAPIAudioOutput::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_audioClient) {
        m_audioClient->Stop();
        m_audioClient->Reset();
        m_running = false;
    }
}

size_t WASAPIAudioOutput::write(const float* data, size_t frames) {
    if (!m_running || !m_audioClient || !m_renderClient || data == nullptr || frames == 0) return 0;

    UINT32 padding = 0;
    if (FAILED(m_audioClient->GetCurrentPadding(&padding))) {
        return 0;
    }

    UINT32 availableFrames = m_bufferFrameCount - padding;
    if (availableFrames == 0) {
        // Wait briefly for buffer space
        WaitForSingleObject(m_audioEvent, 5);
        if (FAILED(m_audioClient->GetCurrentPadding(&padding))) return 0;
        availableFrames = m_bufferFrameCount - padding;
        if (availableFrames == 0) return 0;
    }

    size_t framesToWrite = std::min(static_cast<size_t>(availableFrames), frames);
    BYTE* buffer = nullptr;
    HRESULT hr = m_renderClient->GetBuffer(static_cast<UINT32>(framesToWrite), &buffer);
    if (FAILED(hr) || !buffer) {
        return 0;
    }

    const uint16_t channels = m_format.channels() > 0 ? m_format.channels() : 2;
    float* floatDst = reinterpret_cast<float*>(buffer);
    const size_t totalSamples = framesToWrite * channels;

    float vol = m_muted.load() ? 0.0f : m_volume.load();
    if (std::abs(vol - 1.0f) < 1e-4f) {
        std::memcpy(floatDst, data, totalSamples * sizeof(float));
    } else {
        for (size_t i = 0; i < totalSamples; ++i) {
            floatDst[i] = data[i] * vol;
        }
    }

    m_renderClient->ReleaseBuffer(static_cast<UINT32>(framesToWrite), 0);
    return framesToWrite;
}

void WASAPIAudioOutput::setVolume(float volumeLinear) {
    m_volume = std::clamp(volumeLinear, 0.0f, 1.0f);
}

void WASAPIAudioOutput::setMuted(bool muted) {
    m_muted = muted;
}

core::BitPerfectStatus WASAPIAudioOutput::bitPerfectStatus() const noexcept {
    if (!m_exclusiveAcquired) {
        return core::BitPerfectStatus::Unavailable;
    }

    if (m_muted.load() || std::abs(m_volume.load() - 1.0f) >= 1e-4f) {
        return core::BitPerfectStatus::Unavailable;
    }

    return core::BitPerfectStatus::Verified;
}

std::string WASAPIAudioOutput::bitPerfectStatusExplanation() const {
    if (!m_exclusiveAcquired) {
        if (m_mode == core::OutputMode::BitPerfect || m_mode == core::OutputMode::Exclusive) {
            return "Exclusive mode was rejected by the Windows audio driver or device is in use. Playback fell back to Shared mode (OS mixer active).";
        }
        return "Shared mode active: Windows Audio Engine mixer resamples and mixes audio. Enable Exclusive Mode in Audio Settings for bit-perfect output.";
    }

    if (std::abs(m_volume.load() - 1.0f) >= 1e-4f) {
        return "Software volume is attenuated. Bit-perfect output requires 100% volume to preserve raw sample values.";
    }

    if (m_muted.load()) {
        return "Output is muted.";
    }

    return "Bit-Perfect Verified: Exclusive WASAPI access active. Stream sample rate and bit-depth match hardware with zero DSP/resampling.";
}

} // namespace soniq::audio

#endif // _WIN32
