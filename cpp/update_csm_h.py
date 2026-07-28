def update_csm_h():
    path = 'd:/Unbound/pokemon/cpp/client/network/client_state_manager.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_methods = '''
  void SetCharacterId(uint64_t character_id);
  uint64_t GetCharacterId() const;
  
  void SetSaveStateBlob(const std::vector<uint8_t>& blob);
  std::vector<uint8_t> GetSaveStateBlob() const;
'''
    if 'SetSaveStateBlob' not in content:
        content = content.replace("  void SetCharacterId(uint64_t character_id);\n  uint64_t GetCharacterId() const;", new_methods.strip())
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Updated client_state_manager.h')

update_csm_h()
