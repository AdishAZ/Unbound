def update_packet_h():
    path = 'd:/Unbound/pokemon/cpp/include/network/packet.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_enum = '''  kStoryUpdate,
  kAutosaveAck,
  kUnknown
};'''

    new_enum = '''  kStoryUpdate,
  kAutosaveAck,
  kSaveStateSync,
  kUnknown
};'''

    if old_enum in content:
        content = content.replace(old_enum, new_enum)
        
        struct_def = '''
struct SaveStateSyncPacket {
  uint64_t account_id = 0;
  uint64_t character_id = 0;
  std::vector<uint8_t> save_state_blob;

  std::vector<uint8_t> Serialize() const;
  static SaveStateSyncPacket Deserialize(const std::vector<uint8_t>& payload);
};
'''
        content = content.replace("}  // namespace unboundmp::network", struct_def + "\n}  // namespace unboundmp::network")
        
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Updated packet.h')
    else:
        print('Failed to find old enum in packet.h')

update_packet_h()
