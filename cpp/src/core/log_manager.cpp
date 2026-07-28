#include "core/log_manager.h"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace unboundmp::core {

LogManager& LogManager::Get() {
    static LogManager instance;
    return instance;
}

LogManager::~LogManager() {
    Shutdown();
}

void LogManager::Initialize(const std::string& log_dir, size_t max_file_size_bytes, int max_rotated_files) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logDir = log_dir;
    m_maxFileSizeBytes = max_file_size_bytes;
    m_maxRotatedFiles = max_rotated_files;
    
    m_recentEntries.resize(MAX_RECENT_ENTRIES);
    m_recentEntriesHead = 0;

    if (!fs::exists(m_logDir)) {
        fs::create_directories(m_logDir);
    }

    std::string logFilePath = (fs::path(m_logDir) / "client.log").string();
    m_fileStream.open(logFilePath, std::ios::app);
    if (m_fileStream.is_open()) {
        m_fileStream.seekp(0, std::ios::end);
        m_currentFileSize = m_fileStream.tellp();
    }
}

void LogManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

void LogManager::Log(LogCategory category, LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ring buffer
    m_recentEntries[m_recentEntriesHead] = {now, category, level, message};
    m_recentEntriesHead = (m_recentEntriesHead + 1) % MAX_RECENT_ENTRIES;

    std::string timeStr = FormatTimestamp(now);
    std::string catStr = CategoryToString(category);
    std::string lvlStr = LevelToString(level);

    std::string logLine = std::format("[{}] [{}] [{}] {}\n", timeStr, catStr, lvlStr, message);

    // Console
    if (level == LogLevel::Error || level == LogLevel::Fatal) {
        std::cerr << logLine << std::flush;
    } else {
        std::cout << logLine << std::flush;
    }

    // File
    if (m_fileStream.is_open()) {
        m_fileStream << logLine;
        m_fileStream.flush();
        m_currentFileSize += logLine.size();
        RotateLogFilesIfNeeded();
    }
}

std::vector<LogEntry> LogManager::GetRecentEntries(size_t count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<LogEntry> result;
    size_t fetchCount = std::min(count, MAX_RECENT_ENTRIES);
    result.reserve(fetchCount);
    
    size_t idx = (m_recentEntriesHead + MAX_RECENT_ENTRIES - fetchCount) % MAX_RECENT_ENTRIES;
    for (size_t i = 0; i < fetchCount; ++i) {
        if (m_recentEntries[idx].timestamp.time_since_epoch().count() > 0) {
            result.push_back(m_recentEntries[idx]);
        }
        idx = (idx + 1) % MAX_RECENT_ENTRIES;
    }
    return result;
}

void LogManager::RotateLogFilesIfNeeded() {
    if (m_currentFileSize < m_maxFileSizeBytes) {
        return;
    }

    m_fileStream.close();

    fs::path baseLogPath = fs::path(m_logDir) / "client.log";

    for (int i = m_maxRotatedFiles - 1; i >= 1; --i) {
        fs::path oldPath = fs::path(m_logDir) / std::format("client.{}.log", i);
        fs::path newPath = fs::path(m_logDir) / std::format("client.{}.log", i + 1);
        if (fs::exists(oldPath)) {
            if (i == m_maxRotatedFiles - 1) {
                fs::remove(oldPath);
            } else {
                fs::rename(oldPath, newPath);
            }
        }
    }

    if (fs::exists(baseLogPath)) {
        fs::rename(baseLogPath, fs::path(m_logDir) / "client.1.log");
    }

    m_fileStream.open(baseLogPath.string(), std::ios::out | std::ios::trunc);
    m_currentFileSize = 0;
}

std::string LogManager::FormatTimestamp(const std::chrono::system_clock::time_point& tp) const {
    auto t_c = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t_c), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string LogManager::CategoryToString(LogCategory cat) const {
    switch (cat) {
        case LogCategory::Client: return "Client";
        case LogCategory::Network: return "Network";
        case LogCategory::Database: return "Database";
        case LogCategory::Gameplay: return "Gameplay";
        case LogCategory::UI: return "UI";
        case LogCategory::Error: return "Error";
        default: return "Unknown";
    }
}

std::string LogManager::LevelToString(LogLevel lvl) const {
    switch (lvl) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

} // namespace unboundmp::core
