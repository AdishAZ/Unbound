#pragma once

#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

namespace unboundmp::persistence {

class AutosaveManager {
public:
    using SaveCallback = std::function<void()>;

    AutosaveManager(std::chrono::milliseconds interval, SaveCallback callback);
    ~AutosaveManager();

    void Start();
    void Stop();

private:
    void Run();

    std::chrono::milliseconds interval_;
    SaveCallback callback_;

    std::atomic<bool> running_{false};
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace unboundmp::persistence
