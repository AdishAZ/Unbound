// Package player models an authenticated player session: identity,
// last-known world state, and the connection it's attached to. It sits one
// layer above internal/connection (which knows nothing about players) and
// one layer below internal/network (which routes packets between
// sessions).
package player

import (
	"sync"
	"time"

	"unbound-mp/server/internal/connection"
	"unbound-mp/server/internal/protocol/pb"
)

// ID identifies a player for the lifetime of their session. Distinct from
// connection.ID: today they're assigned together and used almost
// interchangeably, but keeping them separate types avoids baking in an
// assumption that a player can never survive a reconnect in a later
// milestone.
type ID uint32

// Session holds everything the server knows about one connected,
// authenticated player.
type Session struct {
	ID       ID
	Username string
	ConnID   connection.ID

	mu           sync.RWMutex
	state        *pb.PlayerStateUpdate
	follower     *pb.FollowerUpdate
	lastSeen     time.Time
	linkSessionID uint32 // 0 if not currently in a trade/battle link session
}

func NewSession(id ID, username string, connID connection.ID) *Session {
	return &Session{
		ID:       id,
		Username: username,
		ConnID:   connID,
		lastSeen: time.Now(),
	}
}

// UpdateState stores the latest position/movement update for this player.
// Called whenever a PlayerStateUpdate arrives from their client.
func (s *Session) UpdateState(state *pb.PlayerStateUpdate) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.state = state
}

// State returns the last-known state, or nil if the player hasn't sent one
// yet (e.g. immediately after auth, before their first update).
func (s *Session) State() *pb.PlayerStateUpdate {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.state
}

func (s *Session) UpdateFollower(follower *pb.FollowerUpdate) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.follower = follower
}

func (s *Session) Follower() *pb.FollowerUpdate {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.follower
}

// Touch records a heartbeat/liveness signal from this player. See
// internal/network's heartbeat handling.
func (s *Session) Touch() {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.lastSeen = time.Now()
}

// LastSeen returns when Touch (or UpdateState) was last called.
func (s *Session) LastSeen() time.Time {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.lastSeen
}

// LinkSessionID returns the active trade/battle link session this player is
// part of, or 0 if none.
func (s *Session) LinkSessionID() uint32 {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.linkSessionID
}

func (s *Session) SetLinkSessionID(id uint32) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.linkSessionID = id
}

// ToPlayerInfo builds a pb.PlayerInfo snapshot of this session, suitable
// for inclusion in a WorldSnapshot or PlayerJoined packet.
func (s *Session) ToPlayerInfo() *pb.PlayerInfo {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return &pb.PlayerInfo{
		PlayerId: uint32(s.ID),
		Username: s.Username,
		State:    s.state,
		Follower: s.follower,
	}
}
