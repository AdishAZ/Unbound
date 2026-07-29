#include "game_state/game_state.h"
#include "parser/parser_registry.h"
#include "parser/local_player_parser.h"
#include "parser/party_parser.h"
#include "parser/follower_parser.h"
#include "parser/inventory_parser.h"
#include "parser/parse_result.h"

namespace unboundmp::game_state {

void GameState::Update(parser::ParserRegistry& parsers, int64_t frame_count) {
    frame_count_ = frame_count;
    
    if (auto* local_parser = parsers.GetLocalPlayerParser()) {
        auto result = local_parser->Parse(frame_count);
        
        if (result.ok() && result.value.has_value()) {
            const auto& snapshot = *result.value;
            if (!(local_player_.position == snapshot.position)) {
                dirty_flags_ |= kPosition;
            }
            if (!(local_player_.map == snapshot.map)) {
                dirty_flags_ |= kMap;
            }
            if (local_player_.facing != snapshot.facing) {
                dirty_flags_ |= kDirection;
            }
            if (!(local_player_.movement == snapshot.movement)) {
                dirty_flags_ |= kMovement;
            }
            local_player_ = snapshot;
            is_valid_ = true;
        }
    }
    
    if (auto* party_parser = parsers.GetPartyParser()) {
        auto result = party_parser->Parse(frame_count);
        if (result.ok() && result.value.has_value()) {
            const auto& party = *result.value;
            if (party_.count != party.count || party_.slots != party.slots) {
                dirty_flags_ |= kParty;
            }
            party_ = party;
            is_valid_ = true;
        }
    }
    
    if (auto* follower_parser = parsers.GetFollowerParser()) {
        auto result = follower_parser->Parse(frame_count);
        if (result.ok() && result.value.has_value()) {
            const auto& follower = *result.value;
            if (!(follower_ == follower)) {
                dirty_flags_ |= kFollower;
            }
            follower_ = follower;
            is_valid_ = true;
        }
    }

    if (auto* inventory_parser = parsers.GetInventoryParser()) {
        auto result = inventory_parser->Parse(frame_count);
        if (result.ok() && result.value.has_value()) {
            const auto& parsed_inv = *result.value;
            // A robust check would compare every item, but for now we set it dirty on count change 
            // or simply mark dirty to force an update.
            // Let's implement a deep check for inventory_ items.
            bool changed = (inventory_.occupied_slots != parsed_inv.occupied_slots);
            if (!changed) {
                for (size_t i = 0; i < parsed_inv.items.size(); ++i) {
                    if (inventory_.items.size() <= i || 
                        inventory_.items[i].id != parsed_inv.items[i].id || 
                        inventory_.items[i].quantity != parsed_inv.items[i].quantity) {
                        changed = true;
                        break;
                    }
                }
            }
            
            if (changed) {
                dirty_flags_ |= kInventory;
            }
            inventory_ = parsed_inv;
            is_valid_ = true;
        }
    }

    if (auto* event_parser = parsers.GetEventObjectParser()) {
        auto result = event_parser->Parse(frame_count);
        if (result.ok() && result.value.has_value()) {
            const auto& parsed_events = *result.value;
            // For now, any change marks dirty
            bool changed = true; // We can implement deeper diffing later
            if (changed) {
                dirty_flags_ |= kEventObjects;
            }
            event_objects_ = parsed_events;
            is_valid_ = true;
        }
    }
}

} // namespace unboundmp::game_state
