#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <string>
#include "network/packet.h"

namespace unboundmp::core {

enum class EventType {
    kLoginSuccess,
    kLoginFailed,
    kCharacterListReceived,
    kCharacterCreated,
    kCharacterCreationFailed,
    kCharacterSelected,
    kCharacterSelectFailed,
    kCharacterLoaded,
    kWorldEntered,
    kDisconnected,
    kReturnToLoginRequested,
    kSaveAndExitRequested,
    kLogoutResponseReceived
};

struct Event {
    virtual ~Event() = default;
};

struct EmptyEvent : public Event {};

struct LoginFailedEvent : public Event {
    std::string message;
    explicit LoginFailedEvent(std::string msg) : message(std::move(msg)) {}
};

struct CharacterListEvent : public Event {
    std::vector<network::CharacterEntry> characters;
    explicit CharacterListEvent(std::vector<network::CharacterEntry> chars) : characters(std::move(chars)) {}
};

struct CharacterCreatedEvent : public Event {
    uint64_t character_id;
    explicit CharacterCreatedEvent(uint64_t id) : character_id(id) {}
};

struct CharacterCreationFailedEvent : public Event {
    std::string message;
    explicit CharacterCreationFailedEvent(std::string msg) : message(std::move(msg)) {}
};

struct CharacterSelectFailedEvent : public Event {
    std::string message;
    explicit CharacterSelectFailedEvent(std::string msg) : message(std::move(msg)) {}
};

struct CharacterLoadedEvent : public Event {
    bool success;
    std::string message;
    CharacterLoadedEvent(bool s, std::string m) : success(s), message(std::move(m)) {}
};

class EventSystem {
public:
    using EventCallback = std::function<void(const Event&)>;

    static EventSystem& GetInstance() {
        static EventSystem instance;
        return instance;
    }

    uint64_t Subscribe(EventType type, EventCallback callback);
    void Unsubscribe(EventType type, uint64_t id);
    
    // Dispatch immediately
    void Publish(EventType type, const Event& event);
    
    // Thread-safe queueing
    void QueueEvent(EventType type, std::shared_ptr<Event> event);
    void QueueEvent(EventType type); // For empty events
    
    // Process queued events (call from main thread)
    void ProcessEvents();

private:
    EventSystem() = default;
    
    struct QueuedEvent {
        EventType type;
        std::shared_ptr<Event> event;
    };
    
    struct Subscriber {
        uint64_t id;
        EventCallback callback;
    };
    
    std::unordered_map<EventType, std::vector<Subscriber>> subscribers_;
    std::vector<QueuedEvent> event_queue_;
    std::mutex queue_mutex_;
    uint64_t next_id_ = 1;
};

} // namespace unboundmp::core
