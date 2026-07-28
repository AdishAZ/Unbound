def update_char_mgr_h():
    path = 'd:/Unbound/pokemon/cpp/server/characters/character_manager.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_methods = '''
  bool UpdateCharacter(const Character& character);
  bool DeleteCharacter(uint64_t character_id);

  bool UpdateSaveStateBlob(uint64_t character_id, const std::vector<uint8_t>& blob);
  std::optional<std::vector<uint8_t>> GetSaveStateBlob(uint64_t character_id);
'''
    content = content.replace("  bool UpdateCharacter(const Character& character);\n  bool DeleteCharacter(uint64_t character_id);", new_methods.strip())
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Updated character_manager.h')

update_char_mgr_h()
