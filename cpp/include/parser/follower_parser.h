#pragma once
#include "parser/domain_types.h"
#include "parser/parse_result.h"
#include "memory/follower_reader.h"
#include "memory/memory_api.h"
#include "memory/address_table.h"

namespace unboundmp::parser {

class FollowerParser {
public:
    FollowerParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses);
    ParseResult<FollowerInfo> Parse(int64_t frame_count);

private:
    const memory::MemoryApi& memory_;
    const memory::AddressTable& addresses_;
    int64_t last_frame_ = -1;
    ParseResult<FollowerInfo> cached_result_;
};

} // namespace unboundmp::parser
