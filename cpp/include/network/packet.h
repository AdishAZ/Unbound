#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "network/serializer.h"
#include "models/item.h"
#include "models/inventory.h"
namespace unboundmp::network {

enum class PacketType : uint16_t {
  kNone = 0,
  kConnect = 1,
  kDisconnect = 2,
  kPing = 3,
  kPong = 4,
  kHeartbeat = 5,
  
  kAuthRequest = 10,
  kAuthResponse = 11,
  kCreateAccountRequest = 12,
  kLogoutRequest = 13,
  kLogoutResponse = 14,
  
  kCharacterListRequest = 20,
  kCharacterListResponse = 21,
  kCreateCharacterRequest = 22,
  kCreateCharacterResponse = 23,
  kSelectCharacterRequest = 24,
  kSelectCharacterResponse = 25,
  kDeleteCharacterRequest = 26,
  kDeleteCharacterResponse = 27,
  kCharacterLoadedResponse = 28,
  
  kPlayerDataSync = 30,
  kInventoryUpdate = 31,
  kPokemonUpdate = 32,
  kStoryUpdate = 33,
  kAutosaveAck = 34,
  kSaveStateSync = 35,
  
  kPlayerPresence = 40,
  kPlayerState = 41,
  kWorldSnapshot = 42,
  kPlayerSpawn = 43,
  kPlayerDespawn = 44,
  kMapTransition = 45,
  
  kMoveRequest = 46,
  kMoveAccepted = 47,
  kMoveRejected = 48,
  kPlayerMove = 49,
  kPlayerDirection = 50,
  
  kInventoryRequest = 60,
  kInventoryResponse = 61,
  kItemMoved = 62,
  kItemUsed = 63,
  kItemDropped = 64,
  kItemPickup = 65,
  kMoneyUpdated = 66,
  kInventoryError = 67,
  
  kLinkSessionRequest = 70,
  kLinkSessionResponse = 71,
  kLinkSessionData = 72,
  kLinkSessionEnd = 73,
  
  kUnknown = 255
};

struct Packet {
    PacketType type;
    uint32_t size = 0;
    uint64_t sequence_number = 0;
    std::string session_token;
    std::vector<uint8_t> payload;

    void Serialize(Serializer& serializer) const;
    void Deserialize(Serializer& serializer);
    static std::vector<uint8_t> Encode(const Packet& packet);
    static Packet Decode(const std::vector<uint8_t>& data);
};

struct AuthRequestPacket {
    std::string username;
    std::string password;
    std::vector<uint8_t> Serialize() const;
    static AuthRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct AuthResponsePacket {
    bool success;
    std::string message;
    std::string token;
    uint64_t account_id;
    uint64_t character_id;
    std::vector<uint8_t> Serialize() const;
    static AuthResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct LogoutRequestPacket {
    std::vector<uint8_t> Serialize() const;
    static LogoutRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct LogoutResponsePacket {
    bool success;
    std::vector<uint8_t> Serialize() const;
    static LogoutResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

enum class PresenceState : uint8_t {
    kOffline = 0,
    kOnline = 1,
    kPlaying = 2
};

struct PlayerPresencePacket {
    uint64_t account_id;
    uint64_t character_id;
    PresenceState state;
    std::vector<uint8_t> Serialize() const;
    static PlayerPresencePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerStatePacket {
    uint64_t account_id;
    float x;
    float y;
    uint8_t direction;
    uint8_t movement_state;
    std::vector<uint8_t> Serialize() const;
    static PlayerStatePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerData {
    uint64_t account_id;
    uint64_t character_id;
    uint32_t map_id;
    float x;
    float y;
    uint8_t direction;
    uint8_t movement_state;
};

struct WorldEntitySnapshot {
    uint64_t entity_id;
    uint32_t type;
    float x;
    float y;
    uint8_t direction;
};

struct WorldSnapshotPacket {
    uint32_t map_id;
    float spawn_x;
    float spawn_y;
    uint64_t server_tick;
    uint64_t server_time_ms;
    std::vector<PlayerData> players;
    std::vector<WorldEntitySnapshot> placeholder_entities;
    
    std::vector<uint8_t> Serialize() const;
    static WorldSnapshotPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerSpawnPacket {
    uint64_t account_id;
    uint64_t character_id;
    uint32_t map_id;
    float x;
    float y;
    uint8_t direction;
    uint8_t movement_state;
    std::vector<uint8_t> Serialize() const;
    static PlayerSpawnPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerDespawnPacket {
    uint64_t account_id;
    std::vector<uint8_t> Serialize() const;
    static PlayerDespawnPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct MoveRequestPacket {
    float x;
    float y;
    uint8_t direction;
    uint8_t movement_state;
    std::vector<uint8_t> Serialize() const;
    static MoveRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct MoveAcceptedPacket {
    float x;
    float y;
    std::vector<uint8_t> Serialize() const;
    static MoveAcceptedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct MoveRejectedPacket {
    float x;
    float y;
    std::vector<uint8_t> Serialize() const;
    static MoveRejectedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerMovePacket {
    uint64_t account_id;
    float x;
    float y;
    uint8_t movement_state;
    std::vector<uint8_t> Serialize() const;
    static PlayerMovePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerDirectionPacket {
    uint64_t account_id;
    uint8_t direction;
    std::vector<uint8_t> Serialize() const;
    static PlayerDirectionPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct MapTransitionPacket {
    uint64_t account_id;
    uint32_t map_id;
    float x;
    float y;
    std::vector<uint8_t> Serialize() const;
    static MapTransitionPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CreateCharacterRequestPacket {
    std::string name;
    std::string appearance;
    std::vector<uint8_t> Serialize() const;
    static CreateCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CreateCharacterResponsePacket {
    bool success;
    std::string message;
    uint64_t character_id;
    std::vector<uint8_t> Serialize() const;
    static CreateCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct DeleteCharacterRequestPacket {
    uint64_t character_id;
    std::vector<uint8_t> Serialize() const;
    static DeleteCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct DeleteCharacterResponsePacket {
    bool success;
    std::string message;
    std::vector<uint8_t> Serialize() const;
    static DeleteCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CharacterListRequestPacket {
    std::vector<uint8_t> Serialize() const;
    static CharacterListRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CharacterEntry {
    uint64_t id;
    std::string name;
    std::string appearance;
    uint32_t map_id;
    int64_t play_time_seconds;
    uint64_t money;
    int64_t last_login;
};

struct CharacterListResponsePacket {
    std::vector<CharacterEntry> characters;
    std::vector<uint8_t> Serialize() const;
    static CharacterListResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct SelectCharacterRequestPacket {
    uint64_t character_id;
    std::vector<uint8_t> Serialize() const;
    static SelectCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct SelectCharacterResponsePacket {
    bool success;
    std::string message;
    std::vector<uint8_t> Serialize() const;
    static SelectCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CharacterLoadedResponsePacket {
    bool success;
    std::string message;
    std::vector<uint8_t> Serialize() const;
    static CharacterLoadedResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PlayerDataSyncPacket {
    uint64_t account_id;
    uint32_t map_id;
    float x;
    float y;
    uint8_t direction;
    uint32_t money;
    std::vector<uint8_t> inventory_blob;
    std::vector<std::vector<uint8_t>> party_slots;
    std::vector<uint8_t> pc_blob;
    std::vector<uint8_t> story_flags;
    std::vector<uint8_t> story_badges;
    std::vector<uint8_t> story_quests;
    std::vector<uint8_t> Serialize() const;
    static PlayerDataSyncPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct PokemonUpdatePacket {
    uint64_t account_id;
    std::vector<std::vector<uint8_t>> party_slots;
    std::vector<uint8_t> pc_blob;
    std::vector<uint8_t> Serialize() const;
    static PokemonUpdatePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct StoryUpdatePacket {
    uint64_t account_id;
    std::vector<uint8_t> story_flags;
    std::vector<uint8_t> story_badges;
    std::vector<uint8_t> story_quests;
    std::vector<uint8_t> Serialize() const;
    static StoryUpdatePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct AutosaveAckPacket {
    uint64_t account_id;
    std::vector<uint8_t> Serialize() const;
    static AutosaveAckPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct SaveStateSyncPacket {
    uint64_t account_id;
    uint64_t character_id;
    std::vector<uint8_t> save_state_blob;
    std::vector<uint8_t> Serialize() const;
    static SaveStateSyncPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct InventoryRequestPacket {
    std::vector<uint8_t> Serialize() const;
    static InventoryRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct InventoryResponsePacket {
    unboundmp::models::Inventory inventory;
    std::vector<uint8_t> Serialize() const;
    static InventoryResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct InventoryUpdatePacket {
    std::vector<unboundmp::models::Item> changed_items;
    std::vector<uint32_t> removed_slots;
    uint64_t version_number;
    std::vector<uint8_t> Serialize() const;
    static InventoryUpdatePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct ItemMovedPacket {
    uint32_t from_slot;
    uint32_t to_slot;
    uint32_t amount; // For stack splitting
    std::vector<uint8_t> Serialize() const;
    static ItemMovedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct ItemUsedPacket {
    uint32_t slot;
    uint32_t target_entity; // E.g., which pokemon in party
    std::vector<uint8_t> Serialize() const;
    static ItemUsedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct ItemDroppedPacket {
    uint32_t slot;
    uint32_t amount;
    std::vector<uint8_t> Serialize() const;
    static ItemDroppedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct ItemPickupPacket {
    uint32_t map_item_id;
    std::vector<uint8_t> Serialize() const;
    static ItemPickupPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct MoneyUpdatedPacket {
    uint64_t money;
    std::vector<uint8_t> Serialize() const;
    static MoneyUpdatedPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct InventoryErrorPacket {
    std::string message;
    std::vector<uint8_t> Serialize() const;
    static InventoryErrorPacket Deserialize(const std::vector<uint8_t>& payload);
};

// --- Link Session Packets (trade/battle byte relay) ---

enum class LinkSessionMode : uint8_t {
    kUnspecified = 0,
    kTrade = 1,
    kBattle = 2,
};

enum class LinkSessionEndReason : uint8_t {
    kUnspecified = 0,
    kCompleted = 1,
    kCancelled = 2,
    kPeerLeft = 3,
    kTimeout = 4,
};

struct LinkSessionRequestPacket {
    uint64_t target_account_id;
    LinkSessionMode mode;
    std::vector<uint8_t> Serialize() const;
    static LinkSessionRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct LinkSessionResponsePacket {
    uint32_t session_id;
    uint64_t initiator_account_id;
    uint64_t target_account_id;
    LinkSessionMode mode;
    bool accepted;
    std::string reject_reason;
    std::vector<uint8_t> Serialize() const;
    static LinkSessionResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct LinkSessionDataPacket {
    uint32_t session_id;
    uint64_t from_account_id;
    std::vector<uint8_t> payload;
    uint64_t stream_seq;
    std::vector<uint8_t> Serialize() const;
    static LinkSessionDataPacket Deserialize(const std::vector<uint8_t>& data);
};

struct LinkSessionEndPacket {
    uint32_t session_id;
    LinkSessionEndReason reason;
    std::vector<uint8_t> Serialize() const;
    static LinkSessionEndPacket Deserialize(const std::vector<uint8_t>& payload);
};

}  // namespace unboundmp::network
