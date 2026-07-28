#include "gameplay/follower_manager.h"

#include "gameplay/direction.h"

namespace unboundmp::gameplay {

FollowerManager::PlayerFollower& FollowerManager::GetOrCreate(uint32_t player_id) {
  auto it = players_.find(player_id);
  if (it != players_.end()) {
    return it->second;
  }
  auto [inserted_it, ok] = players_.emplace(player_id, PlayerFollower(config_));
  inserted_it->second.visual.player_id = player_id;
  return inserted_it->second;
}

std::optional<protocol::FollowerUpdate> FollowerManager::UpdateLocalFollower(
    uint32_t local_player_id, const memory::FollowerState& state) {
  PlayerFollower& follower = GetOrCreate(local_player_id);
  FollowerVisualState& visual = follower.visual;

  const bool changed = visual.species_id != state.species_id || visual.visible != state.visible ||
                        visual.shiny != state.shiny;
  if (!changed) {
    return std::nullopt;
  }

  visual.species_id = state.species_id;
  visual.visible = state.visible;
  visual.shiny = state.shiny;

  protocol::FollowerUpdate update;
  update.set_player_id(local_player_id);
  update.set_species_id(state.species_id);
  update.set_visible(state.visible);
  update.set_shiny(state.shiny);
  return update;
}

void FollowerManager::OnFollowerUpdate(const protocol::FollowerUpdate& update) {
  PlayerFollower& follower = GetOrCreate(update.player_id());
  follower.visual.species_id = update.species_id();
  follower.visual.visible = update.visible();
  follower.visual.shiny = update.shiny();
}

void FollowerManager::OnPlayerStateUpdate(const protocol::PlayerStateUpdate& update) {
  PlayerFollower& follower = GetOrCreate(update.player_id());

  const bool map_changed =
      follower.has_map && (follower.map_bank != update.map_bank() || follower.map_number != update.map_number());
  const bool first_sighting = !follower.has_map;

  if (map_changed) {
    // The follower never actually walked from the old map to the new one
    // (map transitions are instantaneous, e.g. a door or route edge), so
    // walking it along the old trail would draw a phantom path across an
    // unrelated map. Reset and re-seed instead - same treatment as a
    // fresh sighting, below.
    follower.trail.Reset();
  }

  if (map_changed || first_sighting) {
    // No usable trail yet: rather than drawing the follower exactly on
    // top of the trainer's sprite, place it one tile behind the trainer's
    // current facing direction - a reasonable resting position for
    // "just appeared" that doesn't require any movement history to
    // compute. Subsequent real steps (below) replace this the moment the
    // trainer actually moves.
    const StepVector behind = StepFor(Opposite(update.facing()));
    const int32_t spawn_x = update.x() + behind.dx;
    const int32_t spawn_y = update.y() + behind.dy;

    follower.visual.tile_x = spawn_x;
    follower.visual.tile_y = spawn_y;
    follower.visual.previous_tile_x = spawn_x;
    follower.visual.previous_tile_y = spawn_y;
    follower.visual.facing = update.facing();
    follower.trail.RecordTile(update.x(), update.y(), update.facing());
  } else {
    follower.trail.RecordTile(update.x(), update.y(), update.facing());
    if (!follower.last_is_moving) {
      // Trainer wasn't moving as of the last update and hasn't changed
      // maps - if they're still stationary now, keep the follower facing
      // the same way the trainer is (both stand still facing the same
      // direction). If the trainer *is* moving now, facing is driven by
      // the consumed trail waypoint in Tick() instead, once the follower
      // actually starts stepping toward it.
      follower.visual.facing = update.facing();
    }
  }

  follower.map_bank = update.map_bank();
  follower.map_number = update.map_number();
  follower.has_map = true;
  follower.last_mode = update.movement();
  follower.last_is_moving = update.is_moving();
}

void FollowerManager::Tick(uint32_t elapsed_ms) {
  for (auto& [player_id, follower] : players_) {
    const AnimationTick tick = follower.animator.Tick(follower.last_mode, follower.last_is_moving, elapsed_ms);

    FollowerVisualState& visual = follower.visual;
    visual.step_progress = tick.step_progress;
    visual.frame = tick.frame;
    visual.is_moving = follower.last_is_moving;

    if (tick.completed_step) {
      // The follower just finished walking into visual.tile_x/y - that
      // becomes the new "previous" tile, and the next target is whatever
      // the trail currently says is lag_steps behind the trainer (which
      // may have advanced further while this step was in flight).
      visual.previous_tile_x = visual.tile_x;
      visual.previous_tile_y = visual.tile_y;

      const TrailWaypoint target = follower.trail.FollowerTarget();
      visual.tile_x = target.x;
      visual.tile_y = target.y;
      if (target.direction != protocol::DIRECTION_UNSPECIFIED) {
        visual.facing = target.direction;
      }
    }
  }
}

std::optional<FollowerVisualState> FollowerManager::GetVisualState(uint32_t player_id) const {
  auto it = players_.find(player_id);
  if (it == players_.end()) {
    return std::nullopt;
  }
  return it->second.visual;
}

std::vector<FollowerVisualState> FollowerManager::AllVisualStates() const {
  std::vector<FollowerVisualState> out;
  out.reserve(players_.size());
  for (const auto& [player_id, follower] : players_) {
    out.push_back(follower.visual);
  }
  return out;
}

void FollowerManager::RemovePlayer(uint32_t player_id) { players_.erase(player_id); }

}  // namespace unboundmp::gameplay
