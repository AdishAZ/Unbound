package network

import (
	"sync"
	"sync/atomic"

	"unbound-mp/server/internal/player"
	"unbound-mp/server/internal/protocol/pb"
)

// linkSession represents one active trade or battle byte-relay between
// exactly two players. The server never inspects LinkSessionData.payload -
// see the design note at the top of proto/packets.proto.
type linkSession struct {
	id          uint32
	mode        pb.LinkMode
	initiatorID player.ID
	targetID    player.ID
}

// LinkSessionManager tracks pending requests and active sessions. One
// instance per server process, shared by the Router.
type LinkSessionManager struct {
	nextID atomic.Uint32

	mu       sync.Mutex
	pending  map[uint32]*linkSession // awaiting the target's LinkSessionResponse
	active   map[uint32]*linkSession
}

func NewLinkSessionManager() *LinkSessionManager {
	return &LinkSessionManager{
		pending: make(map[uint32]*linkSession),
		active:  make(map[uint32]*linkSession),
	}
}

// Request registers a new pending session initiated by `from` targeting
// `to`, in the given mode. Returns the session ID to reference in the
// eventual LinkSessionResponse.
func (m *LinkSessionManager) Request(from, to player.ID, mode pb.LinkMode) uint32 {
	id := m.nextID.Add(1)
	m.mu.Lock()
	m.pending[id] = &linkSession{id: id, mode: mode, initiatorID: from, targetID: to}
	m.mu.Unlock()
	return id
}

// Accept promotes a pending session to active. Returns the session and
// true if sessionID was pending; false if it wasn't found (already
// resolved, expired, or never existed - caller should treat that as an
// error to report back to whoever sent the response).
func (m *LinkSessionManager) Accept(sessionID uint32) (*linkSession, bool) {
	m.mu.Lock()
	defer m.mu.Unlock()
	s, ok := m.pending[sessionID]
	if !ok {
		return nil, false
	}
	delete(m.pending, sessionID)
	m.active[sessionID] = s
	return s, true
}

// Reject discards a pending session without activating it.
func (m *LinkSessionManager) Reject(sessionID uint32) (*linkSession, bool) {
	m.mu.Lock()
	defer m.mu.Unlock()
	s, ok := m.pending[sessionID]
	if ok {
		delete(m.pending, sessionID)
	}
	return s, ok
}

// Get returns an active session by ID, if any.
func (m *LinkSessionManager) Get(sessionID uint32) (*linkSession, bool) {
	m.mu.Lock()
	defer m.mu.Unlock()
	s, ok := m.active[sessionID]
	return s, ok
}

// End removes a session (pending or active) by ID, e.g. once
// LinkSessionEnd is sent/received, or when a participant disconnects.
func (m *LinkSessionManager) End(sessionID uint32) {
	m.mu.Lock()
	defer m.mu.Unlock()
	delete(m.pending, sessionID)
	delete(m.active, sessionID)
}

// EndAllForPlayer tears down every pending/active session involving id -
// called on disconnect. Returns the list of ended session IDs paired with
// the other participant, so the caller can notify them.
type endedSession struct {
	sessionID uint32
	otherParty player.ID
}

func (m *LinkSessionManager) EndAllForPlayer(id player.ID) []endedSession {
	m.mu.Lock()
	defer m.mu.Unlock()

	var ended []endedSession
	for sid, s := range m.pending {
		if s.initiatorID == id || s.targetID == id {
			delete(m.pending, sid)
			ended = append(ended, endedSession{sessionID: sid, otherParty: otherParty(s, id)})
		}
	}
	for sid, s := range m.active {
		if s.initiatorID == id || s.targetID == id {
			delete(m.active, sid)
			ended = append(ended, endedSession{sessionID: sid, otherParty: otherParty(s, id)})
		}
	}
	return ended
}

func otherParty(s *linkSession, id player.ID) player.ID {
	if s.initiatorID == id {
		return s.targetID
	}
	return s.initiatorID
}
