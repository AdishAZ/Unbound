#include "network/client_session_manager.h"
#include "core/log_manager.h"
#include "core/event_system.h"
#include "network/client_packet_dispatcher.h"
#include <chrono>

namespace unboundmp::network {

void ClientSessionManager::Initialize() {
    auto& dispatcher = ClientPacketDispatcher::GetInstance();
    
    dispatcher.Subscribe(PacketType::kAuthResponse, [this](const Packet& packet) {
        auto resp = AuthResponsePacket::Deserialize(packet.payload);
        if (resp.success) {
            CreateSession(resp.token, resp.account_id);
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kLoginSuccess,
                std::make_shared<core::EmptyEvent>()
            );
        } else {
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kLoginFailed,
                std::make_shared<core::LoginFailedEvent>(resp.message)
            );
        }
    });
    
    dispatcher.Subscribe(PacketType::kCharacterListResponse, [this](const Packet& packet) {
        auto resp = CharacterListResponsePacket::Deserialize(packet.payload);
        core::EventSystem::GetInstance().QueueEvent(
            core::EventType::kCharacterListReceived,
            std::make_shared<core::CharacterListEvent>(resp.characters)
        );
    });
    
    dispatcher.Subscribe(PacketType::kCreateCharacterResponse, [this](const Packet& packet) {
        auto resp = CreateCharacterResponsePacket::Deserialize(packet.payload);
        if (resp.success) {
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kCharacterCreated,
                std::make_shared<core::CharacterCreatedEvent>(resp.character_id)
            );
        } else {
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kCharacterCreationFailed,
                std::make_shared<core::CharacterCreationFailedEvent>(resp.message)
            );
        }
    });

    dispatcher.Subscribe(PacketType::kSelectCharacterResponse, [this](const Packet& packet) {
        auto resp = SelectCharacterResponsePacket::Deserialize(packet.payload);
        // Wait, how does it know which character was selected?
        // Let's pass the message in the event.
        if (resp.success) {
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kCharacterSelected,
                std::make_shared<core::EmptyEvent>()
            );
        } else {
            core::EventSystem::GetInstance().QueueEvent(
                core::EventType::kCharacterSelectFailed,
                std::make_shared<core::CharacterSelectFailedEvent>(resp.message)
            );
        }
    });

    dispatcher.Subscribe(PacketType::kCharacterLoadedResponse, [this](const Packet& packet) {
        auto resp = CharacterLoadedResponsePacket::Deserialize(packet.payload);
        core::EventSystem::GetInstance().QueueEvent(
            core::EventType::kCharacterLoaded,
            std::make_shared<core::CharacterLoadedEvent>(resp.success, resp.message)
        );
    });
}

void ClientSessionManager::CreateSession(const std::string& token, uint64_t account_id) {
    if (active_session_) {
        core::LogManager::Get().Log(core::LogCategory::Network, core::LogLevel::Warning, "ClientSessionManager: Destroying existing session to create a new one.");
        DestroySession();
    }
    
    active_session_ = std::make_unique<ClientPlayerSession>();
    active_session_->session_token = token;
    active_session_->account_id = account_id;
    active_session_->state = SessionState::kAuthenticated;
    
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    active_session_->login_timestamp = now;
    active_session_->last_heartbeat_timestamp = now;
    
    core::LogManager::Get().Log(core::LogCategory::Network, core::LogLevel::Info, "ClientSessionManager: Session created for account " + std::to_string(account_id));
}

void ClientSessionManager::DestroySession() {
    if (active_session_) {
        core::LogManager::Get().Log(core::LogCategory::Network, core::LogLevel::Info, "ClientSessionManager: Destroying session for account " + std::to_string(active_session_->account_id));
        active_session_.reset();
    }
}

std::optional<ClientPlayerSession> ClientSessionManager::GetSession() const {
    if (active_session_) {
        return *active_session_;
    }
    return std::nullopt;
}

ClientPlayerSession* ClientSessionManager::GetMutableSession() {
    return active_session_.get();
}

void ClientSessionManager::SetActiveCharacter(uint64_t character_id) {
    if (active_session_) {
        active_session_->character_id = character_id;
        active_session_->state = SessionState::kCharacterSelected;
        core::LogManager::Get().Log(core::LogCategory::Network, core::LogLevel::Info, "ClientSessionManager: Character " + std::to_string(character_id) + " attached to session.");
    }
}

void ClientSessionManager::SetSessionState(SessionState state) {
    if (active_session_) {
        active_session_->state = state;
    }
}

bool ClientSessionManager::IsAuthenticated() const {
    return active_session_ && active_session_->state != SessionState::kConnecting && active_session_->state != SessionState::kDisconnected;
}

void ClientSessionManager::UpdateHeartbeat() {
    if (active_session_) {
        active_session_->last_heartbeat_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

} // namespace unboundmp::network
