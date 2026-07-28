#include "persistence/autosave_manager.h"

namespace unboundmp::persistence {

AutosaveManager::AutosaveManager(std::chrono::milliseconds interval, SaveCallback callback)
    : interval_(interval), callback_(std::move(callback)) {}

AutosaveManager::~AutosaveManager() {
    Stop();
}

void AutosaveManager::Start() {
    if (running_.exchange(true)) {
        return;
    }
    thread_ = std::thread(&AutosaveManager::Run, this);
}

void AutosaveManager::Stop() {
    if (!running_.exchange(false)) {
        return;
    }
    cv_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void AutosaveManager::Run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_) {
        if (cv_.wait_for(lock, interval_, [this] { return !running_.load(); })) {
            break; // Stop requested
        }
        
        if (callback_) {
            // Unlock while saving so we don't block Stop() requests
            lock.unlock();
            callback_();
            lock.lock();
        }
    }
}

} // namespace unboundmp::persistence
