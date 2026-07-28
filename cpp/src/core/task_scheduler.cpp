#include "core/task_scheduler.h"

namespace unboundmp::core {

TaskScheduler::TaskScheduler(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back(&TaskScheduler::WorkerLoop, this);
    }
    m_timerThread = std::thread(&TaskScheduler::TimerLoop, this);
}

TaskScheduler::~TaskScheduler() {
    Shutdown();
}

TaskHandle TaskScheduler::Submit(std::function<void()> task) {
    auto handle = m_nextHandle++;
    auto t = std::make_shared<Task>(Task{handle, std::move(task), false});
    
    {
        std::lock_guard<std::mutex> lock(m_cancelMutex);
        m_taskMap[handle] = t;
    }
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_tasks.push(t);
    }
    m_workerCv.notify_one();
    return handle;
}

TaskHandle TaskScheduler::SubmitDelayed(std::function<void()> task, std::chrono::milliseconds delay) {
    auto handle = m_nextHandle++;
    auto executeTime = std::chrono::steady_clock::now() + delay;
    
    {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        m_delayedTasks.push(DelayedTask{handle, std::move(task), executeTime, false});
    }
    m_timerCv.notify_one();
    return handle;
}

void TaskScheduler::Cancel(TaskHandle handle) {
    // Only handles normal tasks in this simplified version
    std::lock_guard<std::mutex> lock(m_cancelMutex);
    auto it = m_taskMap.find(handle);
    if (it != m_taskMap.end()) {
        it->second->cancelled = true;
    }
}

void TaskScheduler::Shutdown() {
    if (m_shutdown.exchange(true)) return;
    
    m_workerCv.notify_all();
    m_timerCv.notify_all();
    
    for (auto& worker : m_workers) {
        if (worker.joinable()) worker.join();
    }
    if (m_timerThread.joinable()) m_timerThread.join();
}

size_t TaskScheduler::GetPendingCount() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_tasks.size();
}

size_t TaskScheduler::GetActiveCount() const {
    return m_activeCount.load();
}

void TaskScheduler::WorkerLoop() {
    while (true) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_workerCv.wait(lock, [this] { return m_shutdown || !m_tasks.empty(); });
            
            if (m_shutdown && m_tasks.empty()) return;
            
            task = m_tasks.front();
            m_tasks.pop();
        }
        
        if (task && !task->cancelled) {
            m_activeCount++;
            task->func();
            m_activeCount--;
        }
        
        {
            std::lock_guard<std::mutex> lock(m_cancelMutex);
            if (task) m_taskMap.erase(task->handle);
        }
    }
}

void TaskScheduler::TimerLoop() {
    while (true) {
        std::function<void()> readyTask;
        {
            std::unique_lock<std::mutex> lock(m_timerMutex);
            if (m_delayedTasks.empty()) {
                m_timerCv.wait(lock, [this] { return m_shutdown || !m_delayedTasks.empty(); });
            } else {
                auto nextTime = m_delayedTasks.top().executeTime;
                m_timerCv.wait_until(lock, nextTime, [this, nextTime] {
                    return m_shutdown || (!m_delayedTasks.empty() && m_delayedTasks.top().executeTime < nextTime);
                });
            }
            
            if (m_shutdown) return;
            
            if (!m_delayedTasks.empty() && std::chrono::steady_clock::now() >= m_delayedTasks.top().executeTime) {
                readyTask = m_delayedTasks.top().func;
                m_delayedTasks.pop();
            }
        }
        
        if (readyTask) {
            Submit(readyTask);
        }
    }
}

} // namespace unboundmp::core
