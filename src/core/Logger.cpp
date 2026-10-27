#include "Logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace soniq::core {

Logger& Logger::instance() noexcept {
    static Logger s_logger;
    return s_logger;
}

Logger::Logger() = default;

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) {
        m_fileStream.flush();
        m_fileStream.close();
    }
}

void Logger::setLevel(LogLevel level) noexcept {
    m_level = level;
}

LogLevel Logger::level() const noexcept {
    return m_level;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    m_fileStream.open(path, std::ios::out | std::ios::app);
}

void Logger::addSink(SinkFn sink) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sinks.push_back(std::move(sink));
}

std::string Logger::currentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif

    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << '.'
       << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string_view Logger::levelToString(LogLevel level) const noexcept {
    switch (level) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
        default:                 return "UNKNOWN";
    }
}

void Logger::write(LogLevel level, std::string_view message) {
    std::string timestamp = currentTimestamp();
    std::string_view lvlStr = levelToString(level);

    std::lock_guard<std::mutex> lock(m_mutex);

    // Format: [2026-10-27 22:30:00.123] [INFO ] message
    std::string line = std::format("[{}] [{}] {}\n", timestamp, lvlStr, message);

    if (level >= LogLevel::Warning) {
        std::cerr << line;
    } else {
        std::cout << line;
    }

    if (m_fileStream.is_open()) {
        m_fileStream << line;
        m_fileStream.flush();
    }

    for (const auto& sink : m_sinks) {
        try {
            sink(level, timestamp, message);
        } catch (...) {
        }
    }
}

} // namespace soniq::core
