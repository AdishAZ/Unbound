#include "gameplay/inventory_manager.h"
#include "network/packet.h"
#include "network/multiplayer_client.h"

#include "network/client_packet_dispatcher.h"
#include "network/client_session_manager.h"
#include "core/game_context.h"
#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "emulator/game_bootstrap.h"

namespace unboundmp::gameplay {

void InventoryManager::Initialize(core::GameContext* ctx, const memory::AddressTable& addresses) {
    auto& dispatcher = unboundmp::network::ClientPacketDispatcher::GetInstance();
    
    dispatcher.Subscribe(unboundmp::network::PacketType::kInventoryResponse,
        [this, addresses](const unboundmp::network::Packet& packet) {
            auto sync_packet = unboundmp::network::InventoryResponsePacket::Deserialize(packet.payload);
            inventory_ = sync_packet.inventory;
            is_synced_ = true;
            WriteToMemory(addresses);
            if (on_updated_) {
                on_updated_();
            }
        });

    dispatcher.Subscribe(unboundmp::network::PacketType::kInventoryUpdate,
        [this, addresses](const unboundmp::network::Packet& packet) {
            auto update_packet = unboundmp::network::InventoryUpdatePacket::Deserialize(packet.payload);
            
            // Apply updates
            for (auto slot : update_packet.removed_slots) {
                // Not perfectly robust, but adequate for example
                for (auto it = inventory_.items.begin(); it != inventory_.items.end(); ++it) {
                    if (it->slot_index == slot) {
                        inventory_.items.erase(it);
                        break;
                    }
                }
            }
            
            for (const auto& new_item : update_packet.changed_items) {
                bool found = false;
                for (auto& item : inventory_.items) {
                    if (item.slot_index == new_item.slot_index) {
                        item = new_item;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    inventory_.items.push_back(new_item);
                }
            }
            
            WriteToMemory(addresses);

            if (on_updated_) {
                on_updated_();
            }
        });
}

void InventoryManager::WriteToMemory(const memory::AddressTable& addresses) {
    auto& memory_api = emulator::GameBootstrap::GetInstance().GetEmulator();
    unboundmp::memory::MemoryApi mem(memory_api);

    const auto base_ptr_symbol = addresses.Get("save_block_1_ptr");
    if (!base_ptr_symbol) return;
    
    uint32_t sb1 = mem.ReadU32(base_ptr_symbol->address);
    if (sb1 == 0) return;
    
    // FireRed inventory items start at offset 0x0290 + 8 from SaveBlock1
    uint32_t inventory_addr = sb1 + 0x0290 + 8;

    // We write exactly 174 * 4 = 696 bytes of zeros first, then overwrite with valid items.
    std::vector<uint8_t> bytes(174 * 4, 0);

    for (const auto& item : inventory_.items) {
        if (item.quantity > 0 && item.slot_index < 174) {
            size_t offset = item.slot_index * 4;
            bytes[offset] = item.id & 0xFF;
            bytes[offset + 1] = (item.id >> 8) & 0xFF;
            bytes[offset + 2] = item.quantity & 0xFF;
            bytes[offset + 3] = (item.quantity >> 8) & 0xFF;
        }
    }

    mem.WriteBytes(inventory_addr, bytes);
}

void InventoryManager::SetInventoryFromLocal(const unboundmp::models::Inventory& inventory) {
    inventory_ = inventory;
    if (on_updated_) {
        on_updated_();
    }
}

void InventoryManager::RequestMoveItem(unboundmp::network::MultiplayerClient* client, uint32_t from_slot, uint32_t to_slot, uint32_t amount) {
    if (!client) return;
    
    unboundmp::network::ItemMovedPacket req;
    req.from_slot = from_slot;
    req.to_slot = to_slot;
    req.amount = amount;
    
    unboundmp::network::Packet p;
    p.type = unboundmp::network::PacketType::kItemMoved;
    if (auto session = unboundmp::network::ClientSessionManager::GetInstance().GetSession()) {
        p.session_token = session->session_token;
    }
    p.payload = req.Serialize();
    client->SendPacket(p);
}

void InventoryManager::RequestUseItem(unboundmp::network::MultiplayerClient* client, uint32_t slot_index, uint32_t target_entity) {
    if (!client) return;
    
    unboundmp::network::ItemUsedPacket req;
    req.slot = slot_index;
    req.target_entity = target_entity;
    
    unboundmp::network::Packet p;
    p.type = unboundmp::network::PacketType::kItemUsed;
    if (auto session = unboundmp::network::ClientSessionManager::GetInstance().GetSession()) {
        p.session_token = session->session_token;
    }
    p.payload = req.Serialize();
    client->SendPacket(p);
}

void InventoryManager::RequestDropItem(unboundmp::network::MultiplayerClient* client, uint32_t slot_index, uint32_t amount) {
    if (!client) return;
    
    unboundmp::network::ItemDroppedPacket req;
    req.slot = slot_index;
    req.amount = amount;
    
    unboundmp::network::Packet p;
    p.type = unboundmp::network::PacketType::kItemDropped;
    if (auto session = unboundmp::network::ClientSessionManager::GetInstance().GetSession()) {
        p.session_token = session->session_token;
    }
    p.payload = req.Serialize();
    client->SendPacket(p);
}

} // namespace unboundmp::gameplay
