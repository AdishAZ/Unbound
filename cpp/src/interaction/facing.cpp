#include "interaction/facing.h"

#include "gameplay/direction.h"
#include "interaction/distance.h"

namespace unboundmp::interaction {

TileCoord GetFacedTile(const PlayerSnapshot& observer) {
  const gameplay::StepVector step = gameplay::StepFor(observer.facing);
  return TileCoord{observer.x + step.dx, observer.y + step.dy};
}

bool IsFacing(const PlayerSnapshot& observer, const PlayerSnapshot& other) {
  // kDown is used as the default/unspecified sentinel - a player with no
  // known facing direction shouldn't be considered "facing" anyone.
  if (!SameMap(observer, other)) {
    return false;
  }
  const TileCoord faced = GetFacedTile(observer);
  return other.x == faced.x && other.y == faced.y;
}

}  // namespace unboundmp::interaction
