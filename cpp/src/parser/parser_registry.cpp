#include "parser/parser_registry.h"

namespace unboundmp::parser {

ParserRegistry::ParserRegistry(const memory::MemoryApi& memory, const memory::AddressTable& addresses) : memory_(memory) {
    local_player_parser_ = std::make_unique<LocalPlayerParser>(memory_, addresses);
    party_parser_ = std::make_unique<PartyParser>(memory_, addresses);
    follower_parser_ = std::make_unique<FollowerParser>(memory_, addresses);
    inventory_parser_ = std::make_unique<InventoryParser>(memory_, addresses);
    event_object_parser_ = std::make_unique<EventObjectParser>(memory_);
}

} // namespace unboundmp::parser
