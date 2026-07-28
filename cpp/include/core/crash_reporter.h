#pragma once
#include <string>
#include <functional>
#include <map>

namespace unboundmp::core {

class CrashReporter {
public:
    static CrashReporter& Get();

    void Install();
    void SetLogDirectory(const std::string& path);
    void SetShutdownCallback(std::function<void()> callback);
    void AddContextInfo(const std::string& key, const std::string& value);

private:
    CrashReporter() = default;
    ~CrashReporter() = default;
    CrashReporter(const CrashReporter&) = delete;
    CrashReporter& operator=(const CrashReporter&) = delete;

    std::string m_logDirectory;
    std::function<void()> m_shutdownCallback;
    std::map<std::string, std::string> m_contextInfo;
};

} // namespace unboundmp::core
