#include "network/packet.h"

namespace unboundmp::network {

void Packet::Serialize(Serializer& serializer) const {
  serializer.WriteU8(static_cast<uint8_t>(type));
  // Payload size
  serializer.WriteU32(static_cast<uint32_t>(payload.size()));
  serializer.WriteU64(sequence_number);
  serializer.WriteString(session_token);
  serializer.WriteBytes(payload);
}

void Packet::Deserialize(Serializer& serializer) {
  type = static_cast<PacketType>(serializer.ReadU8());
  size = serializer.ReadU32();
  sequence_number = serializer.ReadU64();
  session_token = serializer.ReadString();
  payload = serializer.ReadBytes();
}

std::vector<uint8_t> Packet::Encode(const Packet& packet) {
  Serializer serializer;
  packet.Serialize(serializer);
  return serializer.GetBuffer();
}

Packet Packet::Decode(const std::vector<uint8_t>& data) {
  Serializer serializer(data);
  Packet packet;
  packet.Deserialize(serializer);
  return packet;
}

std::vector<uint8_t> AuthRequestPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteString(username);
  serializer.WriteString(password);
  return serializer.GetBuffer();
}

AuthRequestPacket AuthRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  AuthRequestPacket packet;
  packet.username = serializer.ReadString();
  packet.password = serializer.ReadString();
  return packet;
}

std::vector<uint8_t> AuthResponsePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteBool(success);
  serializer.WriteString(message);
  serializer.WriteString(token);
  serializer.WriteU64(account_id);
  serializer.WriteU64(character_id);
  return serializer.GetBuffer();
}

AuthResponsePacket AuthResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
    AuthResponsePacket packet;
    Serializer serializer(payload);
    packet.success = serializer.ReadBool();
    packet.message = serializer.ReadString();
    packet.token = serializer.ReadString();
    packet.account_id = serializer.ReadU64();
    packet.character_id = serializer.ReadU64();
    return packet;
}

std::vector<uint8_t> LogoutRequestPacket::Serialize() const {
    Serializer serializer;
    return serializer.GetBuffer();
}

LogoutRequestPacket LogoutRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
    LogoutRequestPacket packet;
    return packet;
}

std::vector<uint8_t> LogoutResponsePacket::Serialize() const {
    Serializer serializer;
    serializer.WriteBool(success);
    return serializer.GetBuffer();
}

LogoutResponsePacket LogoutResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
    LogoutResponsePacket packet;
    Serializer serializer(payload);
    packet.success = serializer.ReadBool();
    return packet;
}

std::vector<uint8_t> PlayerPresencePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU64(character_id);
  serializer.WriteU8(static_cast<uint8_t>(state));
  return serializer.GetBuffer();
}

PlayerPresencePacket PlayerPresencePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerPresencePacket packet;
  packet.account_id = serializer.ReadU64();
  packet.character_id = serializer.ReadU64();
  packet.state = static_cast<PresenceState>(serializer.ReadU8());
  return packet;
}

std::vector<uint8_t> PlayerStatePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  serializer.WriteU8(direction);
  serializer.WriteU8(movement_state);
  return serializer.GetBuffer();
}

PlayerStatePacket PlayerStatePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerStatePacket packet;
  packet.account_id = serializer.ReadU64();
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  packet.direction = serializer.ReadU8();
  packet.movement_state = serializer.ReadU8();
  return packet;
}

std::vector<uint8_t> WorldSnapshotPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU32(map_id);
  serializer.WriteFloat(spawn_x);
  serializer.WriteFloat(spawn_y);
  serializer.WriteU64(server_tick);
  serializer.WriteU64(server_time_ms);
  
  serializer.WriteU32(static_cast<uint32_t>(players.size()));
  for (const auto& p : players) {
    serializer.WriteU64(p.account_id);
    serializer.WriteU64(p.character_id);
    serializer.WriteU32(p.map_id);
    serializer.WriteFloat(p.x);
    serializer.WriteFloat(p.y);
    serializer.WriteU8(p.direction);
    serializer.WriteU8(p.movement_state);
  }
  
  serializer.WriteU32(static_cast<uint32_t>(placeholder_entities.size()));
  for (const auto& e : placeholder_entities) {
    serializer.WriteU64(e.entity_id);
    serializer.WriteU32(e.type);
    serializer.WriteFloat(e.x);
    serializer.WriteFloat(e.y);
    serializer.WriteU8(e.direction);
  }
  return serializer.GetBuffer();
}

WorldSnapshotPacket WorldSnapshotPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  WorldSnapshotPacket packet;
  packet.map_id = serializer.ReadU32();
  packet.spawn_x = serializer.ReadFloat();
  packet.spawn_y = serializer.ReadFloat();
  packet.server_tick = serializer.ReadU64();
  packet.server_time_ms = serializer.ReadU64();
  
  uint32_t count = serializer.ReadU32();
  for (uint32_t i = 0; i < count; ++i) {
    PlayerData p;
    p.account_id = serializer.ReadU64();
    p.character_id = serializer.ReadU64();
    p.map_id = serializer.ReadU32();
    p.x = serializer.ReadFloat();
    p.y = serializer.ReadFloat();
    p.direction = serializer.ReadU8();
    p.movement_state = serializer.ReadU8();
    packet.players.push_back(p);
  }
  
  uint32_t entity_count = serializer.ReadU32();
  for (uint32_t i = 0; i < entity_count; ++i) {
    WorldEntitySnapshot e;
    e.entity_id = serializer.ReadU64();
    e.type = serializer.ReadU32();
    e.x = serializer.ReadFloat();
    e.y = serializer.ReadFloat();
    e.direction = serializer.ReadU8();
    packet.placeholder_entities.push_back(e);
  }
  return packet;
}

std::vector<uint8_t> PlayerSpawnPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU64(character_id);
  serializer.WriteU32(map_id);
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  serializer.WriteU8(direction);
  serializer.WriteU8(movement_state);
  return serializer.GetBuffer();
}

PlayerSpawnPacket PlayerSpawnPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerSpawnPacket packet;
  packet.account_id = serializer.ReadU64();
  packet.character_id = serializer.ReadU64();
  packet.map_id = serializer.ReadU32();
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  packet.direction = serializer.ReadU8();
  packet.movement_state = serializer.ReadU8();
  return packet;
}

std::vector<uint8_t> PlayerDespawnPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  return serializer.GetBuffer();
}

PlayerDespawnPacket PlayerDespawnPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerDespawnPacket packet;
  packet.account_id = serializer.ReadU64();
  return packet;
}

std::vector<uint8_t> MoveRequestPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  serializer.WriteU8(direction);
  serializer.WriteU8(movement_state);
  return serializer.GetBuffer();
}

MoveRequestPacket MoveRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  MoveRequestPacket packet;
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  packet.direction = serializer.ReadU8();
  packet.movement_state = serializer.ReadU8();
  return packet;
}

std::vector<uint8_t> MoveAcceptedPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  return serializer.GetBuffer();
}

MoveAcceptedPacket MoveAcceptedPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  MoveAcceptedPacket packet;
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  return packet;
}

std::vector<uint8_t> MoveRejectedPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  return serializer.GetBuffer();
}

MoveRejectedPacket MoveRejectedPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  MoveRejectedPacket packet;
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  return packet;
}

std::vector<uint8_t> PlayerMovePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  serializer.WriteU8(movement_state);
  return serializer.GetBuffer();
}

PlayerMovePacket PlayerMovePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerMovePacket packet;
  packet.account_id = serializer.ReadU64();
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  packet.movement_state = serializer.ReadU8();
  return packet;
}

std::vector<uint8_t> PlayerDirectionPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU8(direction);
  return serializer.GetBuffer();
}

PlayerDirectionPacket PlayerDirectionPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer serializer(payload);
  PlayerDirectionPacket packet;
  packet.account_id = serializer.ReadU64();
  packet.direction = serializer.ReadU8();
  return packet;
}

std::vector<uint8_t> MapTransitionPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU32(map_id);
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  return serializer.GetBuffer();
}

MapTransitionPacket MapTransitionPacket::Deserialize(const std::vector<uint8_t>& payload) {
  MapTransitionPacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();
  packet.map_id = serializer.ReadU32();
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  return packet;
}


// --- Character Management Packets ---
std::vector<uint8_t> CreateCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteString(name);
  s.WriteString(appearance);
  return s.GetBuffer();
}
CreateCharacterRequestPacket CreateCharacterRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CreateCharacterRequestPacket p;
  p.name = s.ReadString();
  p.appearance = s.ReadString();
  return p;
}

std::vector<uint8_t> CreateCharacterResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  s.WriteU64(character_id);
  return s.GetBuffer();
}
CreateCharacterResponsePacket CreateCharacterResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CreateCharacterResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  p.character_id = s.ReadU64();
  return p;
}

std::vector<uint8_t> DeleteCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteU64(character_id);
  return s.GetBuffer();
}
DeleteCharacterRequestPacket DeleteCharacterRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  DeleteCharacterRequestPacket p;
  p.character_id = s.ReadU64();
  return p;
}

std::vector<uint8_t> DeleteCharacterResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  return s.GetBuffer();
}
DeleteCharacterResponsePacket DeleteCharacterResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  DeleteCharacterResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  return p;
}

std::vector<uint8_t> CharacterListRequestPacket::Serialize() const {
  return std::vector<uint8_t>();
}
CharacterListRequestPacket CharacterListRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  return CharacterListRequestPacket();
}

std::vector<uint8_t> CharacterListResponsePacket::Serialize() const {
  Serializer s;
  s.WriteU32(static_cast<uint32_t>(characters.size()));
  for (const auto& c : characters) {
    s.WriteU64(c.id);
    s.WriteString(c.name);
    s.WriteString(c.appearance);
    s.WriteU32(c.map_id);
    s.WriteU64(static_cast<uint64_t>(c.play_time_seconds));
    s.WriteU64(c.money);
    s.WriteU64(static_cast<uint64_t>(c.last_login));
  }
  return s.GetBuffer();
}
CharacterListResponsePacket CharacterListResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CharacterListResponsePacket p;
  uint32_t count = s.ReadU32();
  for (uint32_t i = 0; i < count; i++) {
    CharacterEntry c;
    c.id = s.ReadU64();
    c.name = s.ReadString();
    c.appearance = s.ReadString();
    c.map_id = s.ReadU32();
    c.play_time_seconds = static_cast<int64_t>(s.ReadU64());
    c.money = s.ReadU64();
    c.last_login = static_cast<int64_t>(s.ReadU64());
    p.characters.push_back(c);
  }
  return p;
}

std::vector<uint8_t> SelectCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteU64(character_id);
  return s.GetBuffer();
}
SelectCharacterRequestPacket SelectCharacterRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  SelectCharacterRequestPacket p;
  p.character_id = s.ReadU64();
  return p;
}

std::vector<uint8_t> SelectCharacterResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  return s.GetBuffer();
}
SelectCharacterResponsePacket SelectCharacterResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  SelectCharacterResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  return p;
}

std::vector<uint8_t> CharacterLoadedResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  return s.GetBuffer();
}

CharacterLoadedResponsePacket CharacterLoadedResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CharacterLoadedResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  return p;
}

// --- Stage 8 Packets ---


std::vector<uint8_t> PlayerDataSyncPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU32(map_id);
  serializer.WriteFloat(x);
  serializer.WriteFloat(y);
  serializer.WriteU8(direction);
  serializer.WriteU32(money);

  serializer.WriteBytes(inventory_blob);

  serializer.WriteU32(static_cast<uint32_t>(party_slots.size()));
  for (const auto& slot : party_slots) {
    serializer.WriteBytes(slot);
  }

  serializer.WriteBytes(pc_blob);
  serializer.WriteBytes(story_flags);
  serializer.WriteBytes(story_badges);
  serializer.WriteBytes(story_quests);

  return serializer.GetBuffer();
}

PlayerDataSyncPacket PlayerDataSyncPacket::Deserialize(const std::vector<uint8_t>& payload) {
  PlayerDataSyncPacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();
  packet.map_id = serializer.ReadU32();
  packet.x = serializer.ReadFloat();
  packet.y = serializer.ReadFloat();
  packet.direction = serializer.ReadU8();
  packet.money = serializer.ReadU32();

  packet.inventory_blob = serializer.ReadBytes();

  uint32_t party_size = serializer.ReadU32();
  packet.party_slots.resize(party_size);
  for (uint32_t i = 0; i < party_size; ++i) {
    packet.party_slots[i] = serializer.ReadBytes();
  }

  packet.pc_blob = serializer.ReadBytes();
  packet.story_flags = serializer.ReadBytes();
  packet.story_badges = serializer.ReadBytes();
  packet.story_quests = serializer.ReadBytes();

  return packet;
}

std::vector<uint8_t> PokemonUpdatePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  
  serializer.WriteU32(static_cast<uint32_t>(party_slots.size()));
  for (const auto& slot : party_slots) {
    serializer.WriteBytes(slot);
  }

  serializer.WriteBytes(pc_blob);

  return serializer.GetBuffer();
}

PokemonUpdatePacket PokemonUpdatePacket::Deserialize(const std::vector<uint8_t>& payload) {
  PokemonUpdatePacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();

  uint32_t party_size = serializer.ReadU32();
  packet.party_slots.resize(party_size);
  for (uint32_t i = 0; i < party_size; ++i) {
    packet.party_slots[i] = serializer.ReadBytes();
  }

  packet.pc_blob = serializer.ReadBytes();

  return packet;
}

std::vector<uint8_t> StoryUpdatePacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteBytes(story_flags);
  serializer.WriteBytes(story_badges);
  serializer.WriteBytes(story_quests);
  return serializer.GetBuffer();
}

StoryUpdatePacket StoryUpdatePacket::Deserialize(const std::vector<uint8_t>& payload) {
  StoryUpdatePacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();
  packet.story_flags = serializer.ReadBytes();
  packet.story_badges = serializer.ReadBytes();
  packet.story_quests = serializer.ReadBytes();
  return packet;
}

std::vector<uint8_t> AutosaveAckPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  return serializer.GetBuffer();
}

AutosaveAckPacket AutosaveAckPacket::Deserialize(const std::vector<uint8_t>& payload) {
  AutosaveAckPacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();
  return packet;
}


std::vector<uint8_t> SaveStateSyncPacket::Serialize() const {
  Serializer serializer;
  serializer.WriteU64(account_id);
  serializer.WriteU64(character_id);
  serializer.WriteBytes(save_state_blob);
  return serializer.GetBuffer();
}

SaveStateSyncPacket SaveStateSyncPacket::Deserialize(const std::vector<uint8_t>& payload) {
  SaveStateSyncPacket packet;
  Serializer serializer(payload);
  packet.account_id = serializer.ReadU64();
  packet.character_id = serializer.ReadU64();
  packet.save_state_blob = serializer.ReadBytes();
  return packet;
}

// --- Item / Inventory Helpers ---
static void SerializeItem(Serializer& s, const unboundmp::models::Item& item) {
    s.WriteU32(item.id);
    s.WriteU8(static_cast<uint8_t>(item.type));
    s.WriteString(item.name);
    s.WriteU32(item.quantity);
    s.WriteU32(item.slot_index);
    s.WriteU32(item.stack_size);
    s.WriteU64(item.flags);
    s.WriteString(item.metadata);
    s.WriteU64(static_cast<uint64_t>(item.created_at));
}

static unboundmp::models::Item DeserializeItem(Serializer& s) {
    unboundmp::models::Item item;
    item.id = s.ReadU32();
    item.type = static_cast<unboundmp::models::ItemType>(s.ReadU8());
    item.name = s.ReadString();
    item.quantity = s.ReadU32();
    item.slot_index = s.ReadU32();
    item.stack_size = s.ReadU32();
    item.flags = s.ReadU64();
    item.metadata = s.ReadString();
    item.created_at = static_cast<int64_t>(s.ReadU64());
    return item;
}

// --- InventoryRequestPacket ---
std::vector<uint8_t> InventoryRequestPacket::Serialize() const {
    Serializer s;
    return s.GetBuffer();
}

InventoryRequestPacket InventoryRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
    return InventoryRequestPacket();
}

// --- InventoryResponsePacket ---
std::vector<uint8_t> InventoryResponsePacket::Serialize() const {
    Serializer s;
    s.WriteU32(inventory.max_slots);
    s.WriteU32(inventory.occupied_slots);
    s.WriteU64(inventory.money);
    s.WriteU64(inventory.version_number);
    s.WriteBool(inventory.is_dirty);
    s.WriteU32(static_cast<uint32_t>(inventory.items.size()));
    for (const auto& item : inventory.items) {
        SerializeItem(s, item);
    }
    return s.GetBuffer();
}

InventoryResponsePacket InventoryResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
    InventoryResponsePacket p;
    Serializer s(payload);
    p.inventory.max_slots = s.ReadU32();
    p.inventory.occupied_slots = s.ReadU32();
    p.inventory.money = s.ReadU64();
    p.inventory.version_number = s.ReadU64();
    p.inventory.is_dirty = s.ReadBool();
    uint32_t count = s.ReadU32();
    for (uint32_t i = 0; i < count; i++) {
        p.inventory.items.push_back(DeserializeItem(s));
    }
    return p;
}

// --- InventoryUpdatePacket ---
std::vector<uint8_t> InventoryUpdatePacket::Serialize() const {
    Serializer s;
    s.WriteU64(version_number);
    s.WriteU32(static_cast<uint32_t>(changed_items.size()));
    for (const auto& item : changed_items) {
        SerializeItem(s, item);
    }
    s.WriteU32(static_cast<uint32_t>(removed_slots.size()));
    for (auto slot : removed_slots) {
        s.WriteU32(slot);
    }
    return s.GetBuffer();
}

InventoryUpdatePacket InventoryUpdatePacket::Deserialize(const std::vector<uint8_t>& payload) {
    InventoryUpdatePacket p;
    Serializer s(payload);
    p.version_number = s.ReadU64();
    uint32_t item_count = s.ReadU32();
    for (uint32_t i = 0; i < item_count; i++) {
        p.changed_items.push_back(DeserializeItem(s));
    }
    uint32_t rem_count = s.ReadU32();
    for (uint32_t i = 0; i < rem_count; i++) {
        p.removed_slots.push_back(s.ReadU32());
    }
    return p;
}

// --- ItemMovedPacket ---
std::vector<uint8_t> ItemMovedPacket::Serialize() const {
    Serializer s;
    s.WriteU32(from_slot);
    s.WriteU32(to_slot);
    s.WriteU32(amount);
    return s.GetBuffer();
}

ItemMovedPacket ItemMovedPacket::Deserialize(const std::vector<uint8_t>& payload) {
    ItemMovedPacket p;
    Serializer s(payload);
    p.from_slot = s.ReadU32();
    p.to_slot = s.ReadU32();
    p.amount = s.ReadU32();
    return p;
}

// --- ItemUsedPacket ---
std::vector<uint8_t> ItemUsedPacket::Serialize() const {
    Serializer s;
    s.WriteU32(slot);
    s.WriteU32(target_entity);
    return s.GetBuffer();
}

ItemUsedPacket ItemUsedPacket::Deserialize(const std::vector<uint8_t>& payload) {
    ItemUsedPacket p;
    Serializer s(payload);
    p.slot = s.ReadU32();
    p.target_entity = s.ReadU32();
    return p;
}

// --- ItemDroppedPacket ---
std::vector<uint8_t> ItemDroppedPacket::Serialize() const {
    Serializer s;
    s.WriteU32(slot);
    s.WriteU32(amount);
    return s.GetBuffer();
}

ItemDroppedPacket ItemDroppedPacket::Deserialize(const std::vector<uint8_t>& payload) {
    ItemDroppedPacket p;
    Serializer s(payload);
    p.slot = s.ReadU32();
    p.amount = s.ReadU32();
    return p;
}

// --- ItemPickupPacket ---
std::vector<uint8_t> ItemPickupPacket::Serialize() const {
    Serializer s;
    s.WriteU32(map_item_id);
    return s.GetBuffer();
}

ItemPickupPacket ItemPickupPacket::Deserialize(const std::vector<uint8_t>& payload) {
    ItemPickupPacket p;
    Serializer s(payload);
    p.map_item_id = s.ReadU32();
    return p;
}

// --- MoneyUpdatedPacket ---
std::vector<uint8_t> MoneyUpdatedPacket::Serialize() const {
    Serializer s;
    s.WriteU64(money);
    return s.GetBuffer();
}

MoneyUpdatedPacket MoneyUpdatedPacket::Deserialize(const std::vector<uint8_t>& payload) {
    MoneyUpdatedPacket p;
    Serializer s(payload);
    p.money = s.ReadU64();
    return p;
}

// --- InventoryErrorPacket ---
std::vector<uint8_t> InventoryErrorPacket::Serialize() const {
    Serializer s;
    s.WriteString(message);
    return s.GetBuffer();
}

InventoryErrorPacket InventoryErrorPacket::Deserialize(const std::vector<uint8_t>& payload) {
    InventoryErrorPacket p;
    Serializer s(payload);
    p.message = s.ReadString();
    return p;
}

}  // namespace unboundmp::network
