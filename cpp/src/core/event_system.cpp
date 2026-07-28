#include "core/event_system.h"
#include "core/log_manager.h"
#include <algorithm>

namespace unboundmp::core {

uint64_t EventSystem::Subscribe(EventType type, EventCallback callback) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    uint64_t id = next_id_++;
    subscribers_[type].push_back({id, std::move(callback)});
    return id;
}

void EventSystem::Unsubscribe(EventType type, uint64_t id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    auto& list = subscribers_[type];
    list.erase(std::remove_if(list.begin(), list.end(), 
        [id](const Subscriber& s) { return s.id == id; }), list.end());
}

void EventSystem::Publish(EventType type, const Event& event) {
    std::vector<EventCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (subscribers_.find(type) != subscribers_.end()) {
            for (const auto& s : subscribers_[type]) {
                callbacks.push_back(s.callback);
            }
        }
    }
    
    for (const auto& cb : callbacks) {
        cb(event);
    }
}

void EventSystem::QueueEvent(EventType type, std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    event_queue_.push_back({type, std::move(event)});
}

void EventSystem::QueueEvent(EventType type) {
    QueueEvent(type, std::make_shared<EmptyEvent>());
}

void EventSystem::ProcessEvents() {
    std::vector<QueuedEvent> processing_queue;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        processing_queue = std::move(event_queue_);
    }
    
    for (const auto& item : processing_queue) {
        Publish(item.type, *item.event);
    }
}

} // namespace unboundmp::core
