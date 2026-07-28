#pragma once
#include <cstdint>
#include <functional>
#include <chrono>
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>

namespace unboundmp::core {

using TaskHandle = uint64_t;

class TaskScheduler {
public:
    TaskScheduler(size_t numThreads = 2);
    ~TaskScheduler();

    TaskHandle Submit(std::function<void()> task);
    TaskHandle SubmitDelayed(std::function<void()> task, std::chrono::milliseconds delay);
    void Cancel(TaskHandle handle);
    void Shutdown();

    size_t GetPendingCount() const;
    size_t GetActiveCount() const;

private:
    void WorkerLoop();
    void TimerLoop();

    struct Task {
        TaskHandle handle;
        std::function<void()> func;
        bool cancelled = false;
    };

    struct DelayedTask {
        TaskHandle handle;
        std::function<void()> func;
        std::chrono::steady_clock::time_point executeTime;
        bool cancelled = false;
        
        bool operator>(const DelayedTask& other) const {
            return executeTime > other.executeTime;
        }
    };

    std::vector<std::thread> m_workers;
    std::thread m_timerThread;

    mutable std::mutex m_queueMutex;
    std::condition_variable m_workerCv;
    std::queue<std::shared_ptr<Task>> m_tasks;
    
    mutable std::mutex m_timerMutex;
    std::condition_variable m_timerCv;
    std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<DelayedTask>> m_delayedTasks;

    std::atomic<bool> m_shutdown{false};
    std::atomic<TaskHandle> m_nextHandle{1};
    std::atomic<size_t> m_activeCount{0};
    
    // For cancellation
    std::mutex m_cancelMutex;
    std::map<TaskHandle, std::shared_ptr<Task>> m_taskMap;
};

} // namespace unboundmp::core
