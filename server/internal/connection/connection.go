// Package connection owns the raw net.Conn lifecycle for a single client:
// framing reads/writes, a bounded outbound queue, and clean shutdown. It has
// no idea what a "player" is - internal/player builds on top of this.
package connection

import (
	"errors"
	"io"
	"log"
	"net"
	"sync"
	"sync/atomic"

	"unbound-mp/server/internal/config"
	"unbound-mp/server/internal/protocol"
	"unbound-mp/server/internal/protocol/pb"
)

// ID uniquely identifies a connection for the lifetime of the process.
// Assigned by Manager.Accept, monotonically increasing - never reused.
type ID uint64

// DisconnectHandler is invoked exactly once when a connection's read or
// write loop terminates, whatever the cause (peer closed, error, or a call
// to Conn.Close()). reason is nil for a clean peer-initiated close.
type DisconnectHandler func(id ID, reason error)

// Conn wraps one accepted net.Conn with framed Envelope I/O running on two
// dedicated goroutines (read loop, write loop) so that a slow or malicious
// peer can never block the caller's own goroutine.
type Conn struct {
	ID ID

	raw net.Conn

	inbound  chan *pb.Envelope // decoded packets, consumed by the caller
	outbound chan *pb.Envelope // packets queued for sending

	closeOnce sync.Once
	closed    atomic.Bool
	doneCh    chan struct{}

	onDisconnect DisconnectHandler
}

func newConn(id ID, raw net.Conn, onDisconnect DisconnectHandler) *Conn {
	c := &Conn{
		ID:           id,
		raw:          raw,
		inbound:      make(chan *pb.Envelope, config.WriteQueueSize),
		outbound:     make(chan *pb.Envelope, config.WriteQueueSize),
		doneCh:       make(chan struct{}),
		onDisconnect: onDisconnect,
	}
	go c.readLoop()
	go c.writeLoop()
	return c
}

// Inbound returns the channel of decoded packets received from the peer.
// Closed when the connection terminates - range over it, don't just Recv
// once.
func (c *Conn) Inbound() <-chan *pb.Envelope {
	return c.inbound
}

// Send queues env for delivery to the peer. Non-blocking: if the outbound
// queue is full (peer isn't reading fast enough, or is gone), the
// connection is force-closed rather than backing up the caller - see
// config.WriteQueueSize.
func (c *Conn) Send(env *pb.Envelope) {
	if c.closed.Load() {
		return
	}
	select {
	case c.outbound <- env:
	default:
		log.Printf("connection %d: outbound queue full, dropping connection", c.ID)
		c.Close(errors.New("outbound queue overflow"))
	}
}

// Close terminates the connection. Safe to call multiple times and from
// any goroutine; only the first call has effect.
func (c *Conn) Close(reason error) {
	c.closeOnce.Do(func() {
		c.closed.Store(true)
		_ = c.raw.Close()
		close(c.doneCh)
		if c.onDisconnect != nil {
			c.onDisconnect(c.ID, reason)
		}
	})
}

func (c *Conn) readLoop() {
	defer close(c.inbound)
	for {
		env, err := protocol.ReadEnvelope(c.raw)
		if err != nil {
			if errors.Is(err, io.EOF) {
				c.Close(nil) // peer closed cleanly
			} else {
				c.Close(err)
			}
			return
		}
		select {
		case c.inbound <- env:
		case <-c.doneCh:
			return
		}
	}
}

func (c *Conn) writeLoop() {
	for {
		select {
		case env, ok := <-c.outbound:
			if !ok {
				return
			}
			if err := protocol.WriteEnvelope(c.raw, env); err != nil {
				c.Close(err)
				return
			}
		case <-c.doneCh:
			return
		}
	}
}
