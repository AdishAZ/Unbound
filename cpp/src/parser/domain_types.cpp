#include "parser/domain_types.h"

namespace unboundmp::parser {

uint8_t ToNetworkDirection(FacingDirection dir) {
    return static_cast<uint8_t>(dir);
}

FacingDirection FromNetworkDirection(uint8_t dir) {
    switch (dir) {
        case 1: return FacingDirection::kDown;
        case 2: return FacingDirection::kUp;
        case 3: return FacingDirection::kLeft;
        case 4: return FacingDirection::kRight;
        default: return FacingDirection::kDown;
    }
}

uint8_t ToNetworkMovementState(const PlayerMovementState& state) {
    uint8_t val = static_cast<uint8_t>(state.mode);
    if (state.is_moving) {
        val |= 0x80;
    }
    return val;
}

PlayerMovementState FromNetworkMovementState(uint8_t state) {
    PlayerMovementState ms;
    ms.is_moving = (state & 0x80) != 0;
    uint8_t mode = state & 0x7F;
    switch (mode) {
        case 1: ms.mode = MovementMode::kWalk; break;
        case 2: ms.mode = MovementMode::kRun; break;
        case 3: ms.mode = MovementMode::kBike; break;
        case 4: ms.mode = MovementMode::kSurf; break;
        default: ms.mode = MovementMode::kWalk; break;
    }
    return ms;
}

} // namespace unboundmp::parser
