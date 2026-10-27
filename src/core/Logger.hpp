#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <functional>
#include <format>
#include <chrono>

namespace soniq::core {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    using SinkFn = std::function<void(LogLevel level, std::string_view timestamp, std::string_view message)>;

    static Logger& instance() noexcept;

    void setLevel(LogLevel level) noexcept;
    [[nodiscard]] LogLevel level() const noexcept;

    void setLogFile(const std::string& path);
    void addSink(SinkFn sink);

    template <typename... Args>
    void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
        if (level < m_level) return;
        try {
            std::string msg = std::format(fmt, std::forward<Args>(args)...);
            write(level, msg);
        } catch (...) {
            // Safe fallback to prevent throwing from logging
        }
    }

    void write(LogLevel level, std::string_view message);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string currentTimestamp() const;
    std::string_view levelToString(LogLevel level) const noexcept;

    LogLevel m_level{LogLevel::Info};
    std::mutex m_mutex;
    std::ofstream m_fileStream;
    std::vector<SinkFn> m_sinks;
};

#define SONIQ_LOG_TRACE(...) ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Trace, __VA_ARGS__)
#define SONIQ_LOG_DEBUG(...) ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Debug, __VA_ARGS__)
#define SONIQ_LOG_INFO(...)  ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Info, __VA_ARGS__)
#define SONIQ_LOG_WARN(...)  ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Warning, __VA_ARGS__)
#define SONIQ_LOG_ERROR(...) ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Error, __VA_ARGS__)
#define SONIQ_LOG_CRIT(...)  ::soniq::core::Logger::instance().log(::soniq::core::LogLevel::Critical, __VA_ARGS__)

} // namespace soniq::core
