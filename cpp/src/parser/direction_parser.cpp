#include "parser/direction_parser.h"

namespace unboundmp::parser {

DirectionParser::DirectionParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses, const memory::DirectionEncoding& encoding)
    : memory_(memory), addresses_(addresses), encoding_(encoding) {}

ParseResult<FacingDirection> DirectionParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::DirectionReader reader(memory_, addresses_, encoding_);
    auto result = reader.Read();
    if (result.ok()) {
        FacingDirection dir = FacingDirection::kDown;
        switch (static_cast<int>(result.value.value())) {
            case 0: dir = FacingDirection::kDown; break;
            case 1: dir = FacingDirection::kUp; break;
            case 2: dir = FacingDirection::kLeft; break;
            case 3: dir = FacingDirection::kRight; break;
            default: dir = FacingDirection::kDown; break;
        }
        cached_result_ = ParseResult<FacingDirection>::Success(dir, frame_count);
    } else {
        cached_result_ = ParseResult<FacingDirection>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
