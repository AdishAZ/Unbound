#pragma once

#include <memory>
#include "network/packet_dispatcher.h"
#include "server/network/player_registry.h"
#include "server/sessions/session_manager.h"
#include "gameplay/world_sync.h"
#include "gameplay/remote_player_manager.h"
#include "gameplay/sync_scheduler.h"

namespace unboundmp::gameplay {

class GameplaySyncManager {
 public:
  // Server-side initialization
  GameplaySyncManager(std::shared_ptr<server::PlayerRegistry> registry,
                      std::shared_ptr<server::SessionManager> session_manager);

  // Client-side initialization
  explicit GameplaySyncManager(game_state::GameState* state);

  // Register server handlers on the dispatcher
  void RegisterServerHandlers(network::PacketDispatcher& dispatcher);
  
  // Register client handlers on the dispatcher
  void RegisterClientHandlers(network::PacketDispatcher& dispatcher);

  // Returns the Server's world sync component
  WorldSync* GetWorldSync() { return world_sync_.get(); }
  
  // Returns the Client's remote player manager
  RemotePlayerManager* GetRemotePlayerManager() { return remote_player_manager_.get(); }
  
  // Returns the Client's sync scheduler
  SyncScheduler* GetSyncScheduler() { return sync_scheduler_.get(); }

 private:
  // Server dependencies
  std::shared_ptr<server::PlayerRegistry> registry_;
  std::shared_ptr<server::SessionManager> session_manager_;
  std::unique_ptr<WorldSync> world_sync_;
  
  // Client dependencies
  game_state::GameState* state_;
  std::unique_ptr<RemotePlayerManager> remote_player_manager_;
  std::unique_ptr<SyncScheduler> sync_scheduler_;
};

}  // namespace unboundmp::gameplay
