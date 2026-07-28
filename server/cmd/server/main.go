// Command server is the multiplayer backend for the Pokemon Unbound
// wrapper. This milestone implements: connection accept/teardown, simple
// username auth, session tracking, heartbeats, and packet routing for
// world-presence, follower, and trade/battle-link packets. It does not
// touch mGBA, ROM memory, or rendering - see the top-level README.md for
// milestone scope.
package main

import (
	"flag"
	"fmt"
	"log"

	"unbound-mp/server/internal/auth"
	"unbound-mp/server/internal/config"
	"unbound-mp/server/internal/connection"
	"unbound-mp/server/internal/network"
	"unbound-mp/server/internal/player"
)

func main() {
	port := flag.Int("port", config.DefaultPort, "TCP port to listen on")
	maxPlayers := flag.Int("max-players", config.MaxPlayers, "maximum concurrent authenticated players")
	flag.Parse()

	addr := fmt.Sprintf(":%d", *port)

	connMgr, err := connection.NewManager(addr)
	if err != nil {
		log.Fatalf("failed to start listener: %v", err)
	}
	defer connMgr.Close()

	players := player.NewRegistry()
	authReg := auth.NewRegistry(*maxPlayers)

	router := network.NewRouter(connMgr, players, authReg)

	heartbeat := network.NewHeartbeatMonitor(players, router)
	go heartbeat.Run()
	defer heartbeat.Stop()

	log.Printf("unbound-mp server listening on %s (protocol v%d, max %d players)",
		connMgr.Addr(), config.ProtocolVersion, *maxPlayers)

	if err := connMgr.Serve(); err != nil {
		log.Fatalf("server error: %v", err)
	}
}
