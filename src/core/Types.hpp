#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <span>
#include <chrono>

namespace soniq::core {

enum class SampleFormat : uint8_t {
    Unknown = 0,
    Int16,
    Int24,
    Int32,
    Float32,
    Float64,
    DSD64,
    DSD128,
    DSD256
};

enum class ChannelLayout : uint8_t {
    Unknown = 0,
    Mono,
    Stereo,
    Stereo2Point1,
    Quadraphonic,
    Surround5Point1,
    Surround7Point1
};

enum class PlaybackState : uint8_t {
    Stopped = 0,
    Playing,
    Paused,
    Seeking,
    Buffering,
    Error
};

enum class RepeatMode : uint8_t {
    Off = 0,
    Track,
    Queue
};

enum class OutputMode : uint8_t {
    Shared = 0,
    Exclusive,
    BitPerfect
};

enum class BitPerfectStatus : uint8_t {
    Verified = 0,     // Exact bit-match output confirmed
    Available,        // Hardware and format can support it, awaiting activation
    Unavailable,      // DSP, format mismatch, or shared-mode active
    Unknown           // Output backend cannot confirm bitstream integrity
};

struct Error {
    int code{0};
    std::string message;
    std::string module;

    [[nodiscard]] bool hasError() const noexcept { return code != 0 || !message.empty(); }
    [[nodiscard]] static Error ok() noexcept { return Error{0, "", ""}; }
    [[nodiscard]] static Error make(int code, std::string msg, std::string mod = "Core") {
        return Error{code, std::move(msg), std::move(mod)};
    }
};

template <typename T>
class Result {
public:
    Result(T value) : m_data(std::move(value)) {}
    Result(Error error) : m_data(std::move(error)) {}

    [[nodiscard]] bool isOk() const noexcept { return std::holds_alternative<T>(m_data); }
    [[nodiscard]] bool isError() const noexcept { return std::holds_alternative<Error>(m_data); }

    [[nodiscard]] const T& value() const { return std::get<T>(m_data); }
    [[nodiscard]] T& value() { return std::get<T>(m_data); }

    [[nodiscard]] const Error& error() const { return std::get<Error>(m_data); }

    [[nodiscard]] T valueOr(T defaultValue) const {
        return isOk() ? std::get<T>(m_data) : std::move(defaultValue);
    }

private:
    std::variant<T, Error> m_data;
};

// Specialized Result for void
template <>
class Result<void> {
public:
    Result() : m_error(Error::ok()) {}
    Result(Error error) : m_error(std::move(error)) {}

    [[nodiscard]] bool isOk() const noexcept { return !m_error.hasError(); }
    [[nodiscard]] bool isError() const noexcept { return m_error.hasError(); }
    [[nodiscard]] const Error& error() const noexcept { return m_error; }

    static Result<void> ok() noexcept { return Result<void>(); }

private:
    Error m_error;
};

using AudioSample = float;
using AudioBufferSpan = std::span<AudioSample>;
using ConstAudioBufferSpan = std::span<const AudioSample>;

} // namespace soniq::core
