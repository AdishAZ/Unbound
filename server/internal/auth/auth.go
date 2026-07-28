// Package auth implements the simple username-based authentication used in
// this milestone: no password, no token, just a unique, validly-formed
// username per connected session. Usernames are freed on disconnect (see
// Registry.Release), so the same username can reconnect later.
package auth

import (
	"fmt"
	"regexp"
	"sync"

	"unbound-mp/server/internal/config"
)

// usernamePattern allows letters, digits, underscore and hyphen only, to
// keep usernames safe to log, display in-game, and use as map keys
// elsewhere without escaping.
var usernamePattern = regexp.MustCompile(`^[A-Za-z0-9_-]+$`)

// Result mirrors pb.AuthResult without depending on the generated protobuf
// package, so this package has zero dependency on codegen and can be unit
// tested in isolation. internal/network maps Result to pb.AuthResult when
// building the AuthResponse packet.
type Result int

const (
	ResultOK Result = iota
	ResultUsernameTaken
	ResultInvalidName
	ResultServerFull
)

// Registry tracks which usernames are currently in use. One Registry per
// server process.
type Registry struct {
	mu       sync.Mutex
	inUse    map[string]struct{}
	capacity int
}

func NewRegistry(capacity int) *Registry {
	return &Registry{
		inUse:    make(map[string]struct{}),
		capacity: capacity,
	}
}

// ValidateFormat checks username against length/character rules only - it
// does not check availability. Split out so callers can give a fast,
// specific rejection reason before even taking the registry lock.
func ValidateFormat(username string) error {
	if len(username) < config.MinUsernameLength || len(username) > config.MaxUsernameLength {
		return fmt.Errorf("username must be %d-%d characters", config.MinUsernameLength, config.MaxUsernameLength)
	}
	if !usernamePattern.MatchString(username) {
		return fmt.Errorf("username may only contain letters, digits, underscore, and hyphen")
	}
	return nil
}

// TryAcquire attempts to reserve username for a new session. Returns
// ResultOK and reserves the name atomically on success; otherwise returns
// the specific failure reason and reserves nothing.
func (r *Registry) TryAcquire(username string) Result {
	if err := ValidateFormat(username); err != nil {
		return ResultInvalidName
	}

	r.mu.Lock()
	defer r.mu.Unlock()

	if len(r.inUse) >= r.capacity {
		return ResultServerFull
	}
	if _, taken := r.inUse[username]; taken {
		return ResultUsernameTaken
	}

	r.inUse[username] = struct{}{}
	return ResultOK
}

// Release frees username so it can be reused by a future connection.
// Safe to call on a username that was never acquired (no-op).
func (r *Registry) Release(username string) {
	r.mu.Lock()
	defer r.mu.Unlock()
	delete(r.inUse, username)
}
