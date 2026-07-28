def update_csm_h_member():
    path = 'd:/Unbound/pokemon/cpp/client/network/client_state_manager.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'std::vector<uint8_t> save_state_blob_;' not in content:
        content = content.replace("uint64_t character_id_ = 0;", "uint64_t character_id_ = 0;\n  std::vector<uint8_t> save_state_blob_;")
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Added member to client_state_manager.h')

update_csm_h_member()
