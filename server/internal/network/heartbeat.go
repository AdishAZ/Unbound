package network

import (
	"log"
	"time"

	"unbound-mp/server/internal/config"
	"unbound-mp/server/internal/player"
)

// HeartbeatMonitor periodically sends a Ping to every authenticated session
// and disconnects any session that hasn't been heard from (via Pong or any
// other packet - see Session.Touch) within config.HeartbeatTimeout.
type HeartbeatMonitor struct {
	registry *player.Registry
	router   *Router

	stopCh chan struct{}
}

func NewHeartbeatMonitor(registry *player.Registry, router *Router) *HeartbeatMonitor {
	return &HeartbeatMonitor{
		registry: registry,
		router:   router,
		stopCh:   make(chan struct{}),
	}
}

// Run blocks, ticking every config.HeartbeatInterval, until Stop is called.
// Intended to be run on its own goroutine: `go monitor.Run()`.
func (h *HeartbeatMonitor) Run() {
	ticker := time.NewTicker(config.HeartbeatInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			h.tick()
		case <-h.stopCh:
			return
		}
	}
}

func (h *HeartbeatMonitor) Stop() {
	close(h.stopCh)
}

func (h *HeartbeatMonitor) tick() {
	now := time.Now()
	for _, session := range h.registry.All() {
		if now.Sub(session.LastSeen()) > config.HeartbeatTimeout {
			log.Printf("player %d (%s): heartbeat timeout, disconnecting", session.ID, session.Username)
			h.router.DisconnectPlayer(session.ID, "heartbeat timeout")
			continue
		}
		h.router.SendPing(session)
	}
}
