#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>
#include <condition_variable>

#include "emulator/mgba_emulator_core.h"
#include "core/json.h"

namespace unboundmp::persistence {

struct CharacterSaveMetadata {
    uint64_t account_id = 0;
    uint64_t character_id = 0;
    std::string character_name;
    int64_t last_save_timestamp = 0;
    int64_t play_time_seconds = 0;
    int save_version = 1;
};

enum class SaveType {
    kAutoSave,
    kManualSave,
    kExitSave
};

struct SaveRequest {
    SaveType type;
    std::function<void(bool)> on_complete;
};

class ClientSaveManager {
public:
    static ClientSaveManager& GetInstance() {
        static ClientSaveManager instance;
        return instance;
    }

    void Initialize();
    void Shutdown();

    // Sets the active character context for saving/loading
    void SetActiveCharacter(uint64_t account_id, uint64_t character_id, const std::string& character_name);

    // Get resolved paths
    std::string GetSaveDirectory() const;
    std::string GetGameSavePath() const;
    std::string GetAutoSavePath() const;
    std::string GetMetadataPath() const;
    std::string GetBackupDirectory() const;

    // Save Queue API
    void RequestSave(SaveType type, std::function<void(bool)> on_complete = nullptr);
    bool ForceSyncSave();

    // Direct access to metadata
    CharacterSaveMetadata LoadMetadata() const;
    void SaveMetadata(const CharacterSaveMetadata& metadata) const;

    // Load pipeline
    void LoadGameSave(emulator::MgbaEmulatorCore& emulator);
    void LoadSavestate(emulator::MgbaEmulatorCore& emulator);

    // Event-driven autosave hook
    void OnGameEventOccurred();

private:
    ClientSaveManager();
    ~ClientSaveManager();
    
    ClientSaveManager(const ClientSaveManager&) = delete;
    ClientSaveManager& operator=(const ClientSaveManager&) = delete;

    void ProcessSaveQueue();
    bool ExecuteSave(const SaveRequest& request);
    bool VerifyAndCommitSave(const std::string& temp_path, const std::string& final_path);
    void BackupExistingSave(const std::string& path);

    uint64_t active_account_id_ = 0;
    uint64_t active_character_id_ = 0;
    std::string active_character_name_;

    std::queue<SaveRequest> save_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread worker_thread_;
    std::atomic<bool> running_{false};
    
    int64_t play_time_accumulator_ = 0;
    uint32_t last_event_time_ = 0;
};

} // namespace unboundmp::persistence
