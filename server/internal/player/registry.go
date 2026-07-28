package player

import (
	"sync"
	"sync/atomic"

	"unbound-mp/server/internal/connection"
)

// Registry tracks every currently-authenticated Session, indexed both by
// player ID and by the underlying connection ID (so a raw Conn disconnect
// event can be mapped back to a Session without a linear scan).
type Registry struct {
	nextID atomic.Uint32

	mu          sync.RWMutex
	byPlayerID  map[ID]*Session
	byConnID    map[connection.ID]*Session
}

func NewRegistry() *Registry {
	return &Registry{
		byPlayerID: make(map[ID]*Session),
		byConnID:   make(map[connection.ID]*Session),
	}
}

// NextID allocates a new, never-reused player ID. Called once per
// successful authentication.
func (r *Registry) NextID() ID {
	return ID(r.nextID.Add(1))
}

// Add registers a new session. Called immediately after a player
// successfully authenticates.
func (r *Registry) Add(s *Session) {
	r.mu.Lock()
	defer r.mu.Unlock()
	r.byPlayerID[s.ID] = s
	r.byConnID[s.ConnID] = s
}

// Remove unregisters a session by player ID. Called when a player
// disconnects. Returns the removed session (or nil if it wasn't found) so
// the caller can broadcast a PlayerLeft using its data.
func (r *Registry) Remove(id ID) *Session {
	r.mu.Lock()
	defer r.mu.Unlock()
	s, ok := r.byPlayerID[id]
	if !ok {
		return nil
	}
	delete(r.byPlayerID, id)
	delete(r.byConnID, s.ConnID)
	return s
}

// ByID returns the session for a player ID, or nil if not found.
func (r *Registry) ByID(id ID) *Session {
	r.mu.RLock()
	defer r.mu.RUnlock()
	return r.byPlayerID[id]
}

// ByConnID returns the session attached to a given connection, or nil if
// that connection hasn't completed authentication (or has already
// disconnected).
func (r *Registry) ByConnID(connID connection.ID) *Session {
	r.mu.RLock()
	defer r.mu.RUnlock()
	return r.byConnID[connID]
}

// All returns a snapshot slice of every currently-registered session.
// Safe to iterate without holding any lock - callers get their own copy of
// the slice header, though the *Session values themselves are still
// shared and protected by their own internal mutex.
func (r *Registry) All() []*Session {
	r.mu.RLock()
	defer r.mu.RUnlock()
	sessions := make([]*Session, 0, len(r.byPlayerID))
	for _, s := range r.byPlayerID {
		sessions = append(sessions, s)
	}
	return sessions
}

// Count returns the number of currently-authenticated players.
func (r *Registry) Count() int {
	r.mu.RLock()
	defer r.mu.RUnlock()
	return len(r.byPlayerID)
}
