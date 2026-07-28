#include <iostream>
#include <thread>
#include <chrono>

#include "network/multiplayer_server.h"
#include "network/multiplayer_client.h"
#include "network/packet_dispatcher.h"
#include "persistence/autosave_manager.h"
#include "persistence/dirty_flag_manager.h"
#include "persistence/character_service.h"

using namespace unboundmp;

int main() {
    std::cout << "Stage 8 Persistence Test" << std::endl;
    std::cout << "Starting AutosaveManager..." << std::endl;
    
    bool saved = false;
    persistence::AutosaveManager autosave(std::chrono::milliseconds(100), [&]() {
        saved = true;
    });
    
    autosave.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    autosave.Stop();
    
    if (saved) {
        std::cout << "AutosaveManager executed successfully." << std::endl;
    } else {
        std::cout << "AutosaveManager failed to execute." << std::endl;
        return 1;
    }
    
    persistence::DirtyFlagManager dirty_mgr;
    dirty_mgr.MarkDirty(persistence::DirtyComponent::kInventory);
    if (!dirty_mgr.IsDirty(persistence::DirtyComponent::kInventory)) {
        std::cout << "DirtyFlagManager logic failed!" << std::endl;
        return 1;
    }
    
    uint32_t flags = dirty_mgr.GetFlagsAndClear();
    if (flags != static_cast<uint32_t>(persistence::DirtyComponent::kInventory)) {
        std::cout << "DirtyFlagManager flags mismatch!" << std::endl;
        return 1;
    }
    
    if (dirty_mgr.AnyDirty()) {
        std::cout << "DirtyFlagManager failed to clear!" << std::endl;
        return 1;
    }
    
    std::cout << "Persistence mechanics test passed!" << std::endl;
    return 0;
}
