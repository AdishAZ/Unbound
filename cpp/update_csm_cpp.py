def update_csm_cpp():
    path = 'd:/Unbound/pokemon/cpp/client/network/client_state_manager.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_methods = '''
void ClientStateManager::SetSaveStateBlob(const std::vector<uint8_t>& blob) {
  std::lock_guard<std::mutex> lock(mutex_);
  save_state_blob_ = blob;
}

std::vector<uint8_t> ClientStateManager::GetSaveStateBlob() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return save_state_blob_;
}
'''
    if 'SetSaveStateBlob' not in content:
        content = content.replace("}  // namespace unboundmp::client", new_methods + "\n}  // namespace unboundmp::client")
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Updated client_state_manager.cpp')

update_csm_cpp()
