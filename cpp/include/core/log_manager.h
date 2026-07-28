#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <format>
#include <chrono>

namespace unboundmp::core {

enum class LogCategory {
    Client,
    Network,
    Database,
    Gameplay,
    UI,
    Error
};

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogCategory category;
    LogLevel level;
    std::string message;
};

class LogManager {
public:
    static LogManager& Get();

    void Initialize(const std::string& log_dir, size_t max_file_size_bytes, int max_rotated_files);
    void Log(LogCategory category, LogLevel level, const std::string& message);
    std::vector<LogEntry> GetRecentEntries(size_t count) const;
    void Shutdown();

private:
    LogManager() = default;
    ~LogManager();
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    void RotateLogFilesIfNeeded();
    std::string FormatTimestamp(const std::chrono::system_clock::time_point& tp) const;
    std::string CategoryToString(LogCategory cat) const;
    std::string LevelToString(LogLevel lvl) const;

    mutable std::mutex m_mutex;
    std::string m_logDir;
    size_t m_maxFileSizeBytes = 0;
    int m_maxRotatedFiles = 0;
    std::ofstream m_fileStream;
    size_t m_currentFileSize = 0;
    
    std::vector<LogEntry> m_recentEntries;
    size_t m_recentEntriesHead = 0;
    const size_t MAX_RECENT_ENTRIES = 50;
};

// Convenience macros
template <typename... Args>
void LogInfo(LogCategory category, std::format_string<Args...> fmt, Args&&... args) {
    LogManager::Get().Log(category, LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void LogError(LogCategory category, std::format_string<Args...> fmt, Args&&... args) {
    LogManager::Get().Log(category, LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void LogWarning(LogCategory category, std::format_string<Args...> fmt, Args&&... args) {
    LogManager::Get().Log(category, LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...));
}

#define LOG_INFO(category, ...) unboundmp::core::LogInfo(unboundmp::core::LogCategory::category, __VA_ARGS__)
#define LOG_WARN(category, ...) unboundmp::core::LogWarning(unboundmp::core::LogCategory::category, __VA_ARGS__)
#define LOG_ERROR(category, ...) unboundmp::core::LogError(unboundmp::core::LogCategory::category, __VA_ARGS__)

} // namespace unboundmp::core
