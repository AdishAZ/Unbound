#include "parser/follower_parser.h"

namespace unboundmp::parser {

FollowerParser::FollowerParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : memory_(memory), addresses_(addresses) {}

ParseResult<FollowerInfo> FollowerParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::FollowerReader reader(memory_, addresses_);
    auto result = reader.Read();
    if (result.ok()) {
        FollowerInfo info;
        info.species_id = result.value->species_id;
        info.visible = result.value->visible;
        info.shiny = result.value->shiny;
        cached_result_ = ParseResult<FollowerInfo>::Success(info, frame_count);
    } else {
        cached_result_ = ParseResult<FollowerInfo>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
