#include "parser/position_parser.h"
#include "parser/coordinate_normalizer.h"

namespace unboundmp::parser {

PositionParser::PositionParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : memory_(memory), addresses_(addresses) {}

ParseResult<PlayerPosition> PositionParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::PositionReader reader(memory_, addresses_);
    auto result = reader.Read();
    if (result.ok()) {
        PlayerPosition pos{result.value->x, result.value->y};
        CoordinateNormalizer::Normalize(pos.x, pos.y);
        cached_result_ = ParseResult<PlayerPosition>::Success(pos, frame_count);
    } else {
        cached_result_ = ParseResult<PlayerPosition>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
