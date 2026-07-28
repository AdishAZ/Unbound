#pragma once
#include "parser/domain_types.h"
#include "parser/parse_result.h"
#include "memory/movement_state_reader.h"
#include "memory/memory_api.h"
#include "memory/address_table.h"

namespace unboundmp::parser {

class MovementParser {
public:
    MovementParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses, const memory::MovementBitLayout& layout);
    ParseResult<PlayerMovementState> Parse(int64_t frame_count);

private:
    const memory::MemoryApi& memory_;
    const memory::AddressTable& addresses_;
    memory::MovementBitLayout layout_;
    int64_t last_frame_ = -1;
    ParseResult<PlayerMovementState> cached_result_;
};

} // namespace unboundmp::parser
