import re

with open('d:/Unbound/pokemon/cpp/include/network/packet.h', 'r') as f:
    content = f.read()

# Add packet types
types_search = '''  kMapTransition,
  // Stage 8 Persistence Packets'''
types_replace = '''  kMapTransition,
  // Character Management Packets
  kCreateCharacterRequest,
  kCreateCharacterResponse,
  kDeleteCharacterRequest,
  kDeleteCharacterResponse,
  kCharacterListRequest,
  kCharacterListResponse,
  kSelectCharacterRequest,
  kSelectCharacterResponse,
  // Stage 8 Persistence Packets'''
content = content.replace(types_search, types_replace)

# Add structs
structs = '''
// --- Character Management Packets ---
struct CreateCharacterRequestPacket {
  std::string name;
  std::string appearance;
  std::vector<uint8_t> Serialize() const;
  static CreateCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CreateCharacterResponsePacket {
  bool success = false;
  std::string message;
  uint64_t character_id = 0;
  std::vector<uint8_t> Serialize() const;
  static CreateCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct DeleteCharacterRequestPacket {
  uint64_t character_id = 0;
  std::vector<uint8_t> Serialize() const;
  static DeleteCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct DeleteCharacterResponsePacket {
  bool success = false;
  std::string message;
  std::vector<uint8_t> Serialize() const;
  static DeleteCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CharacterListRequestPacket {
  std::vector<uint8_t> Serialize() const;
  static CharacterListRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct CharacterListResponsePacket {
  struct CharacterEntry {
    uint64_t id = 0;
    std::string name;
    std::string appearance;
    uint32_t map_id = 0;
    int64_t play_time_seconds = 0;
    uint64_t money = 0;
    int64_t last_login = 0;
  };
  std::vector<CharacterEntry> characters;
  std::vector<uint8_t> Serialize() const;
  static CharacterListResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

struct SelectCharacterRequestPacket {
  uint64_t character_id = 0;
  std::vector<uint8_t> Serialize() const;
  static SelectCharacterRequestPacket Deserialize(const std::vector<uint8_t>& payload);
};

struct SelectCharacterResponsePacket {
  bool success = false;
  std::string message;
  std::vector<uint8_t> Serialize() const;
  static SelectCharacterResponsePacket Deserialize(const std::vector<uint8_t>& payload);
};

// --- Stage 8 Packets ---
'''
content = content.replace('// --- Stage 8 Packets ---', structs)

with open('d:/Unbound/pokemon/cpp/include/network/packet.h', 'w') as f:
    f.write(content)


with open('d:/Unbound/pokemon/cpp/src/network/packet.cpp', 'r') as f:
    cpp_content = f.read()

cpp_structs = '''
// --- Character Management Packets ---
std::vector<uint8_t> CreateCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteString(name);
  s.WriteString(appearance);
  return s.GetData();
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
  s.WriteUint64(character_id);
  return s.GetData();
}
CreateCharacterResponsePacket CreateCharacterResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CreateCharacterResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  p.character_id = s.ReadUint64();
  return p;
}

std::vector<uint8_t> DeleteCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteUint64(character_id);
  return s.GetData();
}
DeleteCharacterRequestPacket DeleteCharacterRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  DeleteCharacterRequestPacket p;
  p.character_id = s.ReadUint64();
  return p;
}

std::vector<uint8_t> DeleteCharacterResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  return s.GetData();
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
  s.WriteUint32(static_cast<uint32_t>(characters.size()));
  for (const auto& c : characters) {
    s.WriteUint64(c.id);
    s.WriteString(c.name);
    s.WriteString(c.appearance);
    s.WriteUint32(c.map_id);
    s.WriteInt64(c.play_time_seconds);
    s.WriteUint64(c.money);
    s.WriteInt64(c.last_login);
  }
  return s.GetData();
}
CharacterListResponsePacket CharacterListResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  CharacterListResponsePacket p;
  uint32_t count = s.ReadUint32();
  for (uint32_t i = 0; i < count; i++) {
    CharacterEntry c;
    c.id = s.ReadUint64();
    c.name = s.ReadString();
    c.appearance = s.ReadString();
    c.map_id = s.ReadUint32();
    c.play_time_seconds = s.ReadInt64();
    c.money = s.ReadUint64();
    c.last_login = s.ReadInt64();
    p.characters.push_back(c);
  }
  return p;
}

std::vector<uint8_t> SelectCharacterRequestPacket::Serialize() const {
  Serializer s;
  s.WriteUint64(character_id);
  return s.GetData();
}
SelectCharacterRequestPacket SelectCharacterRequestPacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  SelectCharacterRequestPacket p;
  p.character_id = s.ReadUint64();
  return p;
}

std::vector<uint8_t> SelectCharacterResponsePacket::Serialize() const {
  Serializer s;
  s.WriteBool(success);
  s.WriteString(message);
  return s.GetData();
}
SelectCharacterResponsePacket SelectCharacterResponsePacket::Deserialize(const std::vector<uint8_t>& payload) {
  Serializer s(payload);
  SelectCharacterResponsePacket p;
  p.success = s.ReadBool();
  p.message = s.ReadString();
  return p;
}

// --- Stage 8 Packets ---
'''
cpp_content = cpp_content.replace('// --- Stage 8 Packets ---', cpp_structs)

with open('d:/Unbound/pokemon/cpp/src/network/packet.cpp', 'w') as f:
    f.write(cpp_content)

