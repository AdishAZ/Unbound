#include "parser/movement_parser.h"

namespace unboundmp::parser {

MovementParser::MovementParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses, const memory::MovementBitLayout& layout)
    : memory_(memory), addresses_(addresses), layout_(layout) {}

ParseResult<PlayerMovementState> MovementParser::Parse(int64_t frame_count) {
    if (frame_count == last_frame_ && cached_result_.ok()) {
        return cached_result_;
    }

    memory::MovementStateReader reader(memory_, addresses_, layout_);
    auto result = reader.Read();
    if (result.ok()) {
        PlayerMovementState state;
        state.is_moving = false; // Note: actual moving status might be tracked elsewhere or need additional logic
        
        if (result.value->surfing) {
            state.mode = MovementMode::kSurf;
        } else if (result.value->OnBike()) {
            state.mode = MovementMode::kBike;
        } else if (result.value->running) {
            state.mode = MovementMode::kRun;
        } else {
            state.mode = MovementMode::kWalk;
        }
        
        cached_result_ = ParseResult<PlayerMovementState>::Success(state, frame_count);
    } else {
        cached_result_ = ParseResult<PlayerMovementState>::Failure(result.error);
    }
    
    last_frame_ = frame_count;
    return cached_result_;
}

} // namespace unboundmp::parser
