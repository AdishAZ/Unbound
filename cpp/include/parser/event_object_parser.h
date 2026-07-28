#pragma once

#include "parser/domain_types.h"
#include "parser/parse_result.h"
#include "memory/memory_api.h"

namespace unboundmp::parser {

class EventObjectParser {
public:
    EventObjectParser(const memory::MemoryApi& memory);
    
    ParseResult<RawEventObjectData> Parse(int64_t frame_count);

private:
    const memory::MemoryApi& memory_;
    ParseResult<RawEventObjectData> cached_result_;
    int64_t last_frame_ = -1;
};

} // namespace unboundmp::parser
