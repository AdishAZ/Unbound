#include "persistence/client_save_manager.h"
#include "core/log_manager.h"
#include "emulator/game_bootstrap.h"
#include "network/client_session_manager.h"

#include <filesystem>
#include <fstream>
#include <chrono>

namespace unboundmp::persistence {

namespace fs = std::filesystem;

ClientSaveManager::ClientSaveManager() {
}

ClientSaveManager::~ClientSaveManager() {
    Shutdown();
}

void ClientSaveManager::Initialize() {
    if (running_.exchange(true)) return;
    worker_thread_ = std::thread(&ClientSaveManager::ProcessSaveQueue, this);
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "ClientSaveManager initialized.");
}

void ClientSaveManager::Shutdown() {
    if (!running_.exchange(false)) return;
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_cv_.notify_all();
    }
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "ClientSaveManager shut down.");
}

void ClientSaveManager::SetActiveCharacter(uint64_t account_id, uint64_t character_id, const std::string& character_name) {
    active_account_id_ = account_id;
    active_character_id_ = character_id;
    active_character_name_ = character_name;
    
    // Ensure directories exist
    std::error_code ec;
    fs::create_directories(GetSaveDirectory(), ec);
    fs::create_directories(GetBackupDirectory(), ec);
    
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
        "ClientSaveManager: Set active character " + character_name + " (Acct: " + std::to_string(account_id) + ", Char: " + std::to_string(character_id) + ")");
}

std::string ClientSaveManager::GetSaveDirectory() const {
    if (active_account_id_ == 0 || active_character_id_ == 0) return "saves/default";
    return "saves/account_" + std::to_string(active_account_id_) + "/character_" + std::to_string(active_character_id_);
}

std::string ClientSaveManager::GetGameSavePath() const {
    return GetSaveDirectory() + "/game.sav";
}

std::string ClientSaveManager::GetAutoSavePath() const {
    return GetSaveDirectory() + "/autosave.ss1";
}

std::string ClientSaveManager::GetMetadataPath() const {
    return GetSaveDirectory() + "/metadata.json";
}

std::string ClientSaveManager::GetBackupDirectory() const {
    return GetSaveDirectory() + "/backup";
}

void ClientSaveManager::RequestSave(SaveType type, std::function<void(bool)> on_complete) {
    if (active_account_id_ == 0 || active_character_id_ == 0) {
        if (on_complete) on_complete(false);
        return;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    save_queue_.push({type, std::move(on_complete)});
    queue_cv_.notify_one();
}

bool ClientSaveManager::ForceSyncSave() {
    SaveRequest req{SaveType::kExitSave, nullptr};
    return ExecuteSave(req);
}

CharacterSaveMetadata ClientSaveManager::LoadMetadata() const {
    CharacterSaveMetadata meta;
    meta.account_id = active_account_id_;
    meta.character_id = active_character_id_;
    meta.character_name = active_character_name_;
    
    std::string path = GetMetadataPath();
    std::ifstream file(path);
    if (file.is_open()) {
        try {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto json_val = core::JsonParser::Parse(buffer.str());
            if (json_val.IsObject()) {
                auto& json = json_val.AsObject();
                if (json.count("account_id")) meta.account_id = json["account_id"].AsNumber();
                if (json.count("character_id")) meta.character_id = json["character_id"].AsNumber();
                if (json.count("character_name")) meta.character_name = json["character_name"].AsString();
                if (json.count("last_save_timestamp")) meta.last_save_timestamp = json["last_save_timestamp"].AsNumber();
                if (json.count("play_time_seconds")) meta.play_time_seconds = json["play_time_seconds"].AsNumber();
                if (json.count("save_version")) meta.save_version = json["save_version"].AsNumber();
            }
        } catch (...) {
            core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Error, "ClientSaveManager: Failed to parse metadata.json");
        }
    }
    return meta;
}

void ClientSaveManager::SaveMetadata(const CharacterSaveMetadata& meta) const {
    core::JsonObject json;
    json["account_id"] = static_cast<double>(meta.account_id);
    json["character_id"] = static_cast<double>(meta.character_id);
    json["character_name"] = meta.character_name;
    json["last_save_timestamp"] = static_cast<double>(meta.last_save_timestamp);
    json["play_time_seconds"] = static_cast<double>(meta.play_time_seconds);
    json["save_version"] = static_cast<double>(meta.save_version);
    
    std::string path = GetMetadataPath();
    std::string temp_path = path + ".tmp";
    
    std::ofstream file(temp_path);
    if (file.is_open()) {
        file << core::JsonWriter::Write(core::JsonValue(json), 2);
        file.close();
        
        std::error_code ec;
        if (fs::exists(path, ec)) fs::remove(path, ec);
        fs::rename(temp_path, path, ec);
    }
}

void ClientSaveManager::LoadGameSave(emulator::MgbaEmulatorCore& emulator) {
    if (active_account_id_ == 0 || active_character_id_ == 0) return;
    
    std::string game_save = GetGameSavePath();
    std::error_code ec;
    
    // Always load game.sav if it exists (mGBA handles loading it)
    if (fs::exists(game_save, ec)) {
        auto res = emulator.LoadSave(game_save);
        core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
            "3. game.sav loaded | Path: " + game_save + " | Success: " + std::to_string(res.ok) + " | Msg: " + res.message);
    } else {
        // Just touch the file so mGBA uses this path
        std::ofstream touch(game_save, std::ios::binary);
        touch.close();
        auto res = emulator.LoadSave(game_save);
        core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
            "3. game.sav loaded (created empty) | Path: " + game_save + " | Success: " + std::to_string(res.ok) + " | Msg: " + res.message);
    }
}

void ClientSaveManager::LoadSavestate(emulator::MgbaEmulatorCore& emulator) {
    if (active_account_id_ == 0 || active_character_id_ == 0) return;

    std::string auto_save = GetAutoSavePath();
    std::error_code ec;

    // Now apply savestate if it exists
    if (fs::exists(auto_save, ec)) {
        std::string rom_path = "cpp/client/Pokemon Unbound.gba"; // Could be passed in, assuming standard
        fs::path target_ss1 = fs::path(rom_path).replace_extension(".ss1");
        
        fs::copy_file(auto_save, target_ss1, fs::copy_options::overwrite_existing, ec);
        if (!ec) {
            auto state_res = emulator.LoadState(1);
            core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, 
                "4. autosave.ss1 loaded | Path: " + auto_save + " (copied to " + target_ss1.string() + ") | Success: " + std::to_string(state_res.ok) + " | Msg: " + state_res.message);
        } else {
            core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Error, "4. autosave.ss1 failed to copy | Error: " + ec.message());
        }
    } else {
        core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "4. autosave.ss1 skipped (not found) | Path: " + auto_save);
    }
}

void ClientSaveManager::OnGameEventOccurred() {
    // Basic event-driven autosave hook
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (now - last_event_time_ > 30) {
        last_event_time_ = now;
        RequestSave(SaveType::kAutoSave);
    }
}

void ClientSaveManager::ProcessSaveQueue() {
    while (running_) {
        SaveRequest request;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this]() { return !save_queue_.empty() || !running_; });
            
            if (!running_ && save_queue_.empty()) break;
            
            request = save_queue_.front();
            save_queue_.pop();
        }
        
        bool success = ExecuteSave(request);
        if (request.on_complete) {
            request.on_complete(success);
        }
    }
}

bool ClientSaveManager::ExecuteSave(const SaveRequest& request) {
    if (active_account_id_ == 0 || active_character_id_ == 0) return false;
    
    auto& bootstrap = emulator::GameBootstrap::GetInstance();
    auto& emulator = bootstrap.GetEmulator();
    if (!emulator.IsRunning() && !emulator.IsPaused()) return false;
    
    std::string game_save = GetGameSavePath();
    std::string auto_save = GetAutoSavePath();
    
    core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Info, "ClientSaveManager: Executing save...");
    
    // For SaveStates (autosave.ss1), mgba writes to RomName.ss1 when we call SaveState(1)
    bool ss1_success = emulator.SaveState(1).ok;
    
    std::error_code ec;
    if (ss1_success) {
        std::string rom_path = "cpp/client/Pokemon Unbound.gba";
        fs::path source_ss1 = fs::path(rom_path).replace_extension(".ss1");
        
        if (fs::exists(source_ss1, ec)) {
            BackupExistingSave(auto_save);
            VerifyAndCommitSave(source_ss1.string(), auto_save);
        }
    }
    
    // We update metadata
    CharacterSaveMetadata meta = LoadMetadata();
    meta.last_save_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    SaveMetadata(meta);
    
    // Verify game.sav
    if (fs::exists(game_save, ec) && fs::file_size(game_save, ec) > 0) {
        return true;
    } else {
        core::LogManager::Get().Log(core::LogCategory::Client, core::LogLevel::Error, "ClientSaveManager: game.sav is missing or zero size!");
        return false;
    }
}

bool ClientSaveManager::VerifyAndCommitSave(const std::string& temp_path, const std::string& final_path) {
    std::error_code ec;
    if (!fs::exists(temp_path, ec)) return false;
    if (fs::file_size(temp_path, ec) == 0) return false;
    
    if (fs::exists(final_path, ec)) {
        fs::remove(final_path, ec);
    }
    fs::copy_file(temp_path, final_path, fs::copy_options::overwrite_existing, ec);
    return !ec;
}

void ClientSaveManager::BackupExistingSave(const std::string& path) {
    std::error_code ec;
    if (!fs::exists(path, ec)) return;
    
    auto now = std::chrono::system_clock::now().time_since_epoch();
    std::string timestamp = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now).count());
    
    fs::path p(path);
    std::string backup_path = GetBackupDirectory() + "/" + p.filename().string() + "." + timestamp + ".bak";
    fs::copy_file(path, backup_path, fs::copy_options::overwrite_existing, ec);
    
    // Clean up old backups (keep last 5) - simplified for now
}

} // namespace unboundmp::persistence
