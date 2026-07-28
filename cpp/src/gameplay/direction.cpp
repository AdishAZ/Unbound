#include "gameplay/direction.h"

namespace unboundmp::gameplay {

StepVector StepFor(protocol::Direction direction) {
  switch (direction) {
    case protocol::DIRECTION_DOWN:
      return StepVector{0, 1};
    case protocol::DIRECTION_UP:
      return StepVector{0, -1};
    case protocol::DIRECTION_LEFT:
      return StepVector{-1, 0};
    case protocol::DIRECTION_RIGHT:
      return StepVector{1, 0};
    case protocol::DIRECTION_UNSPECIFIED:
    default:
      return StepVector{0, 0};
  }
}

protocol::Direction Opposite(protocol::Direction direction) {
  switch (direction) {
    case protocol::DIRECTION_DOWN:
      return protocol::DIRECTION_UP;
    case protocol::DIRECTION_UP:
      return protocol::DIRECTION_DOWN;
    case protocol::DIRECTION_LEFT:
      return protocol::DIRECTION_RIGHT;
    case protocol::DIRECTION_RIGHT:
      return protocol::DIRECTION_LEFT;
    case protocol::DIRECTION_UNSPECIFIED:
    default:
      return protocol::DIRECTION_UNSPECIFIED;
  }
}

protocol::Direction ToProtocolDirection(memory::FacingDirection direction) {
  switch (direction) {
    case memory::FacingDirection::kSouth:
      return protocol::DIRECTION_DOWN;
    case memory::FacingDirection::kNorth:
      return protocol::DIRECTION_UP;
    case memory::FacingDirection::kWest:
      return protocol::DIRECTION_LEFT;
    case memory::FacingDirection::kEast:
      return protocol::DIRECTION_RIGHT;
    case memory::FacingDirection::kUnknown:
    default:
      return protocol::DIRECTION_UNSPECIFIED;
  }
}

}  // namespace unboundmp::gameplay
