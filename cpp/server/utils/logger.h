#pragma once

#include <string>
#include <iostream>

namespace unboundmp::server {

enum class LogLevel {
  kInfo,
  kWarning,
  kError,
  kDebug
};

class Logger {
 public:
  static void Log(LogLevel level, const std::string& message) {
    const char* prefix = "[INFO]";
    switch (level) {
      case LogLevel::kInfo: prefix = "[INFO]"; break;
      case LogLevel::kWarning: prefix = "[WARN]"; break;
      case LogLevel::kError: prefix = "[ERROR]"; break;
      case LogLevel::kDebug: prefix = "[DEBUG]"; break;
    }
    
    if (level == LogLevel::kError || level == LogLevel::kWarning) {
      std::cerr << prefix << " Server: " << message << std::endl;
    } else {
      std::cout << prefix << " Server: " << message << std::endl;
    }
  }

  static void Info(const std::string& msg) { Log(LogLevel::kInfo, msg); }
  static void Warn(const std::string& msg) { Log(LogLevel::kWarning, msg); }
  static void Error(const std::string& msg) { Log(LogLevel::kError, msg); }
  static void Debug(const std::string& msg) { Log(LogLevel::kDebug, msg); }
};

}  // namespace unboundmp::server
