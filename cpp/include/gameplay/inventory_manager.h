#pragma once
#include "models/inventory.h"
#include <functional>
#include <vector>

namespace unboundmp::network {
    class MultiplayerClient;
}

namespace unboundmp::core {
    class GameContext;
}

namespace unboundmp::memory {
    class AddressTable;
}

namespace unboundmp::gameplay {

class InventoryManager {
public:
    InventoryManager() = default;
    ~InventoryManager() = default;

    void Initialize(core::GameContext* ctx, const memory::AddressTable& addresses);
    void WriteToMemory(const memory::AddressTable& addresses);

    // Called when the local parser reads a new state
    void SetInventoryFromLocal(const unboundmp::models::Inventory& inventory);

    bool IsSynced() const { return is_synced_; }

    const unboundmp::models::Inventory& GetInventory() const { return inventory_; }

    // Requests to the server
    void RequestMoveItem(unboundmp::network::MultiplayerClient* client, uint32_t from_slot, uint32_t to_slot, uint32_t amount);
    void RequestUseItem(unboundmp::network::MultiplayerClient* client, uint32_t slot_index, uint32_t target_entity);
    void RequestDropItem(unboundmp::network::MultiplayerClient* client, uint32_t slot_index, uint32_t amount);
    
    // Callbacks for UI updates
    using OnInventoryUpdated = std::function<void()>;
    void SetOnInventoryUpdatedCallback(OnInventoryUpdated callback) { on_updated_ = std::move(callback); }

private:
    unboundmp::models::Inventory inventory_;
    OnInventoryUpdated on_updated_;
    bool is_synced_ = false;
};

} // namespace unboundmp::gameplay
