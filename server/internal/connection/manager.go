package connection

import (
	"fmt"
	"log"
	"net"
	"sync"
	"sync/atomic"
)

// Manager owns the TCP listener and the set of currently-connected
// (but not necessarily authenticated) connections. It knows nothing about
// players, auth, or game state - see internal/player for what sits on top
// of an authenticated Conn.
type Manager struct {
	listener net.Listener

	nextID   atomic.Uint64
	mu       sync.Mutex
	conns    map[ID]*Conn
	closing  atomic.Bool

	// OnAccept is invoked for every newly-accepted connection, before any
	// data has been read from it. Typically used to kick off the
	// handshake/auth flow. Must be set before calling Serve.
	OnAccept func(c *Conn)

	// OnDisconnect is invoked once a connection is fully torn down (after
	// it has been removed from the manager's internal set). reason is nil
	// for a clean peer-initiated close.
	OnDisconnect func(id ID, reason error)
}

// NewManager creates a Manager listening on addr (e.g. ":7777"). Does not
// start accepting connections until Serve is called.
func NewManager(addr string) (*Manager, error) {
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		return nil, fmt.Errorf("connection: listen on %s: %w", addr, err)
	}
	return &Manager{
		listener: ln,
		conns:    make(map[ID]*Conn),
	}, nil
}

// Addr returns the address the manager is listening on. Useful when addr
// was passed as ":0" (OS-assigned port), e.g. in tests.
func (m *Manager) Addr() net.Addr {
	return m.listener.Addr()
}

// Serve blocks accepting connections until Close is called, at which point
// it returns nil. Every accepted connection is wrapped and handed to
// OnAccept on its own goroutine-backed Conn (see newConn) - Serve itself
// never blocks on any individual connection.
func (m *Manager) Serve() error {
	for {
		raw, err := m.listener.Accept()
		if err != nil {
			// A closed listener surfaces as an error from Accept; treat
			// that as a clean shutdown (triggered by our own Close()),
			// not a failure worth propagating.
			if m.closing.Load() {
				return nil
			}
			return fmt.Errorf("connection: accept: %w", err)
		}

		id := ID(m.nextID.Add(1))
		conn := newConn(id, raw, m.handleDisconnect)

		m.mu.Lock()
		m.conns[id] = conn
		m.mu.Unlock()

		log.Printf("connection %d: accepted from %s", id, raw.RemoteAddr())

		if m.OnAccept != nil {
			m.OnAccept(conn)
		}
	}
}

func (m *Manager) handleDisconnect(id ID, reason error) {
	m.mu.Lock()
	delete(m.conns, id)
	m.mu.Unlock()

	if reason != nil {
		log.Printf("connection %d: disconnected: %v", id, reason)
	} else {
		log.Printf("connection %d: disconnected", id)
	}

	if m.OnDisconnect != nil {
		m.OnDisconnect(id, reason)
	}
}

// Get returns the live connection for id, if any. Returns nil if the
// connection has since disconnected or never existed.
func (m *Manager) Get(id ID) *Conn {
	m.mu.Lock()
	defer m.mu.Unlock()
	return m.conns[id]
}

// Count returns the number of currently-connected sockets (authenticated
// or not).
func (m *Manager) Count() int {
	m.mu.Lock()
	defer m.mu.Unlock()
	return len(m.conns)
}

// Close stops accepting new connections and force-closes every currently
// open connection. Does not wait for their goroutines to fully exit.
func (m *Manager) Close() error {
	m.closing.Store(true)
	err := m.listener.Close()

	m.mu.Lock()
	conns := make([]*Conn, 0, len(m.conns))
	for _, c := range m.conns {
		conns = append(conns, c)
	}
	m.mu.Unlock()

	for _, c := range conns {
		c.Close(nil)
	}
	return err
}

