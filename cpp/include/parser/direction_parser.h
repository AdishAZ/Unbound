#pragma once
#include "parser/domain_types.h"
#include "parser/parse_result.h"
#include "memory/direction_reader.h"
#include "memory/memory_api.h"
#include "memory/address_table.h"

namespace unboundmp::parser {

class DirectionParser {
public:
    DirectionParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses, const memory::DirectionEncoding& encoding);
    ParseResult<FacingDirection> Parse(int64_t frame_count);

private:
    const memory::MemoryApi& memory_;
    const memory::AddressTable& addresses_;
    memory::DirectionEncoding encoding_;
    int64_t last_frame_ = -1;
    ParseResult<FacingDirection> cached_result_;
};

} // namespace unboundmp::parser
