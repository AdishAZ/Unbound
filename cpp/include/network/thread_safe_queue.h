#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace unboundmp::network {

template <typename T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() = default;
  
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

  void Push(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(std::move(item));
    cond_var_.notify_one();
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  std::optional<T> TryPop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T item = std::move(queue_.front());
    queue_.pop_front();
    return item;
  }

  void WaitAndPop(T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this]() { return !queue_.empty(); });
    item = std::move(queue_.front());
    queue_.pop_front();
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable cond_var_;
  std::deque<T> queue_;
};

}  // namespace unboundmp::network
