#pragma once
#include "parser/local_player_parser.h"
#include "parser/party_parser.h"
#include "parser/follower_parser.h"
#include "parser/inventory_parser.h"
#include "parser/event_object_parser.h"
#include "memory/memory_api.h"
#include "memory/address_table.h"
#include <memory>

namespace unboundmp::parser {

class ParserRegistry {
public:
    ParserRegistry(const memory::MemoryApi& memory, const memory::AddressTable& addresses);

    LocalPlayerParser* GetLocalPlayerParser() const { return local_player_parser_.get(); }
    PartyParser* GetPartyParser() const { return party_parser_.get(); }
    FollowerParser* GetFollowerParser() const { return follower_parser_.get(); }
    InventoryParser* GetInventoryParser() const { return inventory_parser_.get(); }
    EventObjectParser* GetEventObjectParser() const { return event_object_parser_.get(); }

    memory::MemoryApi memory_;
    std::unique_ptr<LocalPlayerParser> local_player_parser_;
    std::unique_ptr<PartyParser> party_parser_;
    std::unique_ptr<FollowerParser> follower_parser_;
    std::unique_ptr<InventoryParser> inventory_parser_;
    std::unique_ptr<EventObjectParser> event_object_parser_;
};

} // namespace unboundmp::parser
