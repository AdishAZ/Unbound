def update_packet_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/network/packet.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_impl = '''
std::vector<uint8_t> SaveStateSyncPacket::Serialize() const {
  std::vector<uint8_t> data;
  Serializer serializer(data);
  serializer.Write(account_id);
  serializer.Write(character_id);
  
  uint32_t blob_len = static_cast<uint32_t>(save_state_blob.size());
  serializer.Write(blob_len);
  for (uint8_t b : save_state_blob) serializer.Write(b);
  
  return data;
}

SaveStateSyncPacket SaveStateSyncPacket::Deserialize(const std::vector<uint8_t>& payload) {
  SaveStateSyncPacket packet;
  std::vector<uint8_t> mutable_payload = payload;
  Serializer serializer(mutable_payload);
  
  packet.account_id = serializer.Read<uint64_t>();
  packet.character_id = serializer.Read<uint64_t>();
  
  uint32_t blob_len = serializer.Read<uint32_t>();
  packet.save_state_blob.resize(blob_len);
  for (uint32_t i = 0; i < blob_len; i++) {
    packet.save_state_blob[i] = serializer.Read<uint8_t>();
  }
  
  return packet;
}
'''
    content = content.replace("}  // namespace unboundmp::network", new_impl + "\n}  // namespace unboundmp::network")
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Updated packet.cpp')

update_packet_cpp()
