#include "parser/map_parser.h"

namespace unboundmp::parser {

MapParser::MapParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : memory_(memory), addresses_(addresses) {}

ParseResult<MapLocation> MapParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::MapReader reader(memory_, addresses_);
    auto result = reader.Read();
    if (result.ok()) {
        MapLocation loc{result.value->bank, result.value->number};
        cached_result_ = ParseResult<MapLocation>::Success(loc, frame_count);
    } else {
        cached_result_ = ParseResult<MapLocation>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
