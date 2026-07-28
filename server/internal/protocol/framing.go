// Package protocol provides the wire framing used to read and write
// Envelope messages over a net.Conn. The Envelope/packet message types
// themselves are generated from proto/packets.proto into the pb
// subpackage - see proto/README.md for the codegen command. This file
// only implements the [length-prefix][payload] framing around them, the
// Go-side mirror of cpp/src/protocol/serialization.cpp.
package protocol

import (
	"encoding/binary"
	"fmt"
	"io"

	"google.golang.org/protobuf/proto"

	"unbound-mp/server/internal/config"
	"unbound-mp/server/internal/protocol/pb"
)

// ErrFrameTooLarge is returned when a peer sends (or we're asked to send) a
// frame whose payload exceeds config.MaxFrameBytes. The caller should treat
// this as fatal for the connection - the stream cannot be resynchronized.
var ErrFrameTooLarge = fmt.Errorf("protocol: frame exceeds max size of %d bytes", config.MaxFrameBytes)

// WriteEnvelope serializes env and writes it to w as a single
// [4-byte big-endian length][protobuf payload] frame.
func WriteEnvelope(w io.Writer, env *pb.Envelope) error {
	payload, err := proto.Marshal(env)
	if err != nil {
		return fmt.Errorf("protocol: marshal envelope: %w", err)
	}
	if len(payload) == 0 || len(payload) > config.MaxFrameBytes {
		return ErrFrameTooLarge
	}

	var lengthPrefix [config.FrameLengthPrefixBytes]byte
	binary.BigEndian.PutUint32(lengthPrefix[:], uint32(len(payload)))

	if _, err := w.Write(lengthPrefix[:]); err != nil {
		return fmt.Errorf("protocol: write length prefix: %w", err)
	}
	if _, err := w.Write(payload); err != nil {
		return fmt.Errorf("protocol: write payload: %w", err)
	}
	return nil
}

// ReadEnvelope blocks until it has read one complete frame from r and
// returns the decoded Envelope. Returns io.EOF (or an error wrapping it) if
// the connection is closed cleanly before a frame is available, and
// ErrFrameTooLarge if the peer's declared length exceeds config.MaxFrameBytes
// (the caller should close the connection in that case - the stream is
// desynced and unrecoverable).
func ReadEnvelope(r io.Reader) (*pb.Envelope, error) {
	var lengthPrefix [config.FrameLengthPrefixBytes]byte
	if _, err := io.ReadFull(r, lengthPrefix[:]); err != nil {
		return nil, err
	}

	payloadLen := binary.BigEndian.Uint32(lengthPrefix[:])
	if payloadLen == 0 || payloadLen > config.MaxFrameBytes {
		return nil, ErrFrameTooLarge
	}

	payload := make([]byte, payloadLen)
	if _, err := io.ReadFull(r, payload); err != nil {
		return nil, fmt.Errorf("protocol: read payload: %w", err)
	}

	env := &pb.Envelope{}
	if err := proto.Unmarshal(payload, env); err != nil {
		return nil, fmt.Errorf("protocol: unmarshal envelope: %w", err)
	}
	return env, nil
}
