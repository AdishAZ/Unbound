def update_char_mgr_cpp():
    path = 'd:/Unbound/pokemon/cpp/server/characters/character_manager.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    impl = '''
bool CharacterManager::UpdateSaveStateBlob(uint64_t character_id, const std::vector<uint8_t>& blob) {
    try {
        auto conn = db_pool_->GetConnection();
        pqxx::work txn(*conn);
        
        pqxx::binarystring bin_data(blob.data(), blob.size());
        txn.exec_params(
            "UPDATE characters SET save_state_blob =  WHERE id = ",
            bin_data,
            character_id
        );
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to update save state blob: " + std::string(e.what()));
        return false;
    }
}

std::optional<std::vector<uint8_t>> CharacterManager::GetSaveStateBlob(uint64_t character_id) {
    try {
        auto conn = db_pool_->GetConnection();
        pqxx::work txn(*conn);
        
        auto result = txn.exec_params(
            "SELECT save_state_blob FROM characters WHERE id =  AND save_state_blob IS NOT NULL",
            character_id
        );
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        auto field = result[0][0];
        if (field.is_null()) return std::nullopt;
        
        pqxx::binarystring bin_data(field);
        return std::vector<uint8_t>(bin_data.begin(), bin_data.end());
    } catch (const std::exception& e) {
        Logger::Error("Failed to get save state blob: " + std::string(e.what()));
        return std::nullopt;
    }
}
'''
    content = content.replace("}  // namespace unboundmp::server", impl + "\n}  // namespace unboundmp::server")
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Updated character_manager.cpp')

update_char_mgr_cpp()
