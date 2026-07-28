#pragma once
#include "parser/domain_types.h"
#include "parser/parse_result.h"
#include "parser/position_parser.h"
#include "parser/map_parser.h"
#include "parser/direction_parser.h"
#include "parser/movement_parser.h"
#include "memory/memory_api.h"
#include "memory/address_table.h"
#include <memory>

namespace unboundmp::parser {

class LocalPlayerParser {
public:
    LocalPlayerParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses);
    ParseResult<LocalPlayerSnapshot> Parse(int64_t frame_count);

private:
    const memory::MemoryApi& memory_;
    const memory::AddressTable& addresses_;
    std::unique_ptr<PositionParser> position_parser_;
    std::unique_ptr<MapParser> map_parser_;
    std::unique_ptr<DirectionParser> direction_parser_;
    std::unique_ptr<MovementParser> movement_parser_;
};

} // namespace unboundmp::parser
