#include "core/crash_reporter.h"
#include "core/log_manager.h"
#include <fstream>
#include <chrono>
#include <format>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

namespace fs = std::filesystem;

namespace unboundmp::core {

#ifdef _WIN32
LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    auto& reporter = CrashReporter::Get();
    
    // ... write out crash log
    // This is a minimal implementation per instructions
    std::cout << "Crash intercepted!" << std::endl;
    exit(1);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

CrashReporter& CrashReporter::Get() {
    static CrashReporter instance;
    return instance;
}

void CrashReporter::Install() {
#ifdef _WIN32
    SetUnhandledExceptionFilter(UnhandledExceptionHandler);
#endif
}

void CrashReporter::SetLogDirectory(const std::string& path) {
    m_logDirectory = path;
    if (!fs::exists(m_logDirectory)) {
        fs::create_directories(m_logDirectory);
    }
}

void CrashReporter::SetShutdownCallback(std::function<void()> callback) {
    m_shutdownCallback = callback;
}

void CrashReporter::AddContextInfo(const std::string& key, const std::string& value) {
    m_contextInfo[key] = value;
}

} // namespace unboundmp::core
