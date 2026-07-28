// Package network wires connection.Manager, auth.Registry, and
// player.Registry together: it runs the handshake/auth flow for each new
// connection, dispatches inbound packets by type, and handles disconnect
// cleanup. This is the "packet dispatcher" and "packet routing" deliverable
// for the server, and the Go-side counterpart to
// cpp/include/protocol/packet_dispatcher.h (the two aren't shared code -
// client and server dispatch for different reasons - but they follow the
// same "route by oneof case" shape).
package network

import (
	"log"
	"time"

	"unbound-mp/server/internal/auth"
	"unbound-mp/server/internal/config"
	"unbound-mp/server/internal/connection"
	"unbound-mp/server/internal/player"
	"unbound-mp/server/internal/protocol/pb"
)

// Router is the top-level object gluing every server subsystem together.
// Create one per process; wire its methods to a connection.Manager's
// OnAccept/OnDisconnect before calling Manager.Serve().
type Router struct {
	connMgr  *connection.Manager
	players  *player.Registry
	authReg  *auth.Registry
	links    *LinkSessionManager
}

func NewRouter(connMgr *connection.Manager, players *player.Registry, authReg *auth.Registry) *Router {
	r := &Router{
		connMgr: connMgr,
		players: players,
		authReg: authReg,
		links:   NewLinkSessionManager(),
	}
	connMgr.OnAccept = r.handleAccept
	connMgr.OnDisconnect = r.handleConnDisconnect
	return r
}

// --- Connection lifecycle -------------------------------------------------

func (r *Router) handleAccept(c *connection.Conn) {
	go r.runConnection(c)
}

// runConnection drives one connection end to end: handshake, auth, then
// the main inbound-packet dispatch loop. Returns when the connection's
// inbound channel is closed (i.e. the connection has terminated).
func (r *Router) runConnection(c *connection.Conn) {
	session, ok := r.handshakeAndAuth(c)
	if !ok {
		return // handshake/auth failed; runConnection already closed c
	}

	log.Printf("player %d (%s): authenticated on connection %d", session.ID, session.Username, c.ID)

	r.sendWorldSnapshot(session, c)
	r.broadcastExcept(session.ID, envelopePlayerJoined(session.ToPlayerInfo()))

	for env := range c.Inbound() {
		session.Touch()
		r.dispatch(session, c, env)
	}

	// c.Inbound() closed => connection terminated. Cleanup happens in
	// handleConnDisconnect (registered with connection.Manager), which
	// fires independently of this loop exiting, so we don't duplicate it
	// here.
}

// handshakeAndAuth reads ClientHello then AuthRequest off c, enforcing
// config.HandshakeTimeout for the whole exchange. Returns the created
// session and true on success. On any failure it sends an explanatory
// packet (where applicable) and closes c itself, returning (nil, false).
func (r *Router) handshakeAndAuth(c *connection.Conn) (*player.Session, bool) {
	deadline := time.After(config.HandshakeTimeout)

	// 1. ClientHello
	var clientHello *pb.ClientHello
	select {
	case env, ok := <-c.Inbound():
		if !ok {
			return nil, false
		}
		hello, ok := env.GetPayload().(*pb.Envelope_ClientHello)
		if !ok {
			r.rejectHandshake(c, "expected ClientHello")
			return nil, false
		}
		clientHello = hello.ClientHello
	case <-deadline:
		c.Close(errTimeout("handshake"))
		return nil, false
	}

	if clientHello.GetClientProtocolVersion() != config.ProtocolVersion {
		c.Send(&pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload: &pb.Envelope_ServerHello{ServerHello: &pb.ServerHello{
				Accepted:              false,
				ServerProtocolVersion: config.ProtocolVersion,
				ServerVersionString:   config.ServerVersionString,
				RejectReason:          "protocol version mismatch",
			}},
		})
		c.Close(errProtocolMismatch)
		return nil, false
	}

	c.Send(&pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_ServerHello{ServerHello: &pb.ServerHello{
			Accepted:              true,
			ServerProtocolVersion: config.ProtocolVersion,
			ServerVersionString:   config.ServerVersionString,
		}},
	})

	// 2. AuthRequest
	var authReq *pb.AuthRequest
	select {
	case env, ok := <-c.Inbound():
		if !ok {
			return nil, false
		}
		req, ok := env.GetPayload().(*pb.Envelope_AuthRequest)
		if !ok {
			r.rejectHandshake(c, "expected AuthRequest")
			return nil, false
		}
		authReq = req.AuthRequest
	case <-deadline:
		c.Close(errTimeout("auth"))
		return nil, false
	}

	result := r.authReg.TryAcquire(authReq.GetUsername())
	if result != auth.ResultOK {
		c.Send(&pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload: &pb.Envelope_AuthResponse{AuthResponse: &pb.AuthResponse{
				Result: toPBAuthResult(result),
			}},
		})
		c.Close(errAuthFailed)
		return nil, false
	}

	playerID := r.players.NextID()
	session := player.NewSession(playerID, authReq.GetUsername(), c.ID)
	r.players.Add(session)

	c.Send(&pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_AuthResponse{AuthResponse: &pb.AuthResponse{
			Result:   pb.AuthResult_AUTH_RESULT_OK,
			PlayerId: uint32(playerID),
		}},
	})

	return session, true
}

func (r *Router) rejectHandshake(c *connection.Conn, reason string) {
	c.Send(&pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_Error{Error: &pb.ErrorMessage{
			Code:    pb.ErrorCode_ERROR_CODE_MALFORMED_PACKET,
			Message: reason,
		}},
	})
	c.Close(errProtocolMismatch)
}

// --- Disconnect handling ---------------------------------------------------

// handleConnDisconnect is registered as connection.Manager.OnDisconnect. It
// fires for every torn-down connection, authenticated or not, so it must
// tolerate there being no player.Session for this connection yet.
func (r *Router) handleConnDisconnect(connID connection.ID, reason error) {
	session := r.players.ByConnID(connID)
	if session == nil {
		return // disconnected during handshake, before a session existed
	}

	r.players.Remove(session.ID)
	r.authReg.Release(session.Username)

	for _, ended := range r.links.EndAllForPlayer(session.ID) {
		r.sendToPlayer(ended.otherParty, &pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload: &pb.Envelope_LinkSessionEnd{LinkSessionEnd: &pb.LinkSessionEnd{
				SessionId: ended.sessionID,
				Reason:    pb.LinkSessionEndReason_LINK_SESSION_END_REASON_PEER_LEFT,
			}},
		})
	}

	log.Printf("player %d (%s): removed", session.ID, session.Username)

	r.broadcastExcept(session.ID, &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_PlayerLeft{PlayerLeft: &pb.PlayerLeft{
			PlayerId: uint32(session.ID),
			Reason:   pb.DisconnectReason_DISCONNECT_REASON_CLIENT_QUIT,
		}},
	})
}

// DisconnectPlayer force-closes a player's connection, e.g. from
// HeartbeatMonitor on timeout. handleConnDisconnect performs the actual
// cleanup once the connection's teardown fires.
func (r *Router) DisconnectPlayer(id player.ID, reason string) {
	session := r.players.ByID(id)
	if session == nil {
		return
	}
	if c := r.connMgr.Get(session.ConnID); c != nil {
		c.Close(errTimeout(reason))
	}
}

// --- Inbound packet dispatch -----------------------------------------------

// dispatch routes one inbound Envelope from an already-authenticated
// session. This is the server-side packet dispatcher: every case below
// corresponds to one oneof field in Envelope.
func (r *Router) dispatch(session *player.Session, c *connection.Conn, env *pb.Envelope) {
	switch p := env.GetPayload().(type) {

	case *pb.Envelope_Ping:
		c.Send(&pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload:         &pb.Envelope_Pong{Pong: &pb.Pong{Nonce: p.Ping.GetNonce()}},
		})

	case *pb.Envelope_Pong:
		// session.Touch() already ran in runConnection's loop; nothing
		// further to do.

	case *pb.Envelope_PlayerStateUpdate:
		update := p.PlayerStateUpdate
		update.PlayerId = uint32(session.ID) // never trust the client's own claimed ID
		session.UpdateState(update)
		r.broadcastExcept(session.ID, &pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload:         &pb.Envelope_PlayerStateUpdate{PlayerStateUpdate: update},
		})

	case *pb.Envelope_FollowerUpdate:
		update := p.FollowerUpdate
		update.PlayerId = uint32(session.ID)
		session.UpdateFollower(update)
		r.broadcastExcept(session.ID, &pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload:         &pb.Envelope_FollowerUpdate{FollowerUpdate: update},
		})

	case *pb.Envelope_LinkSessionRequest:
		r.handleLinkSessionRequest(session, p.LinkSessionRequest)

	case *pb.Envelope_LinkSessionResponse:
		r.handleLinkSessionResponse(session, p.LinkSessionResponse)

	case *pb.Envelope_LinkSessionData:
		r.handleLinkSessionData(session, p.LinkSessionData)

	case *pb.Envelope_LinkSessionEnd:
		r.handleLinkSessionEnd(session, p.LinkSessionEnd)

	case *pb.Envelope_Disconnect:
		c.Close(nil)

	default:
		log.Printf("player %d (%s): unhandled packet type %T", session.ID, session.Username, p)
	}
}

// --- Link session (trade/battle relay) handlers ----------------------------

func (r *Router) handleLinkSessionRequest(from *player.Session, req *pb.LinkSessionRequest) {
	targetID := player.ID(req.GetTargetPlayerId())
	target := r.players.ByID(targetID)
	if target == nil {
		r.sendToPlayer(from.ID, envelopeError(pb.ErrorCode_ERROR_CODE_UNKNOWN_PLAYER, "target player not found"))
		return
	}

	sessionID := r.links.Request(from.ID, targetID, req.GetMode())

	// Forward the request to the target as a LinkSessionResponse with
	// Accepted left false - the target's client is expected to prompt the
	// player and reply with its own LinkSessionResponse (Accepted=true) to
	// actually activate the session. We reuse LinkSessionResponse here as
	// the "incoming request" notification so the target doesn't need a
	// distinct packet type to react to.
	r.sendToPlayer(targetID, &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_LinkSessionResponse{LinkSessionResponse: &pb.LinkSessionResponse{
			SessionId:         sessionID,
			InitiatorPlayerId: uint32(from.ID),
			TargetPlayerId:    uint32(targetID),
			Mode:              req.GetMode(),
			Accepted:          false, // "pending", not yet accepted by target
		}},
	})
}

func (r *Router) handleLinkSessionResponse(from *player.Session, resp *pb.LinkSessionResponse) {
	if !resp.GetAccepted() {
		r.links.Reject(resp.GetSessionId())
		r.sendToPlayer(player.ID(resp.GetInitiatorPlayerId()), &pb.Envelope{
			ProtocolVersion: config.ProtocolVersion,
			Payload: &pb.Envelope_LinkSessionResponse{LinkSessionResponse: &pb.LinkSessionResponse{
				SessionId:      resp.GetSessionId(),
				TargetPlayerId: uint32(from.ID),
				Accepted:       false,
				RejectReason:   resp.GetRejectReason(),
			}},
		})
		return
	}

	session, ok := r.links.Accept(resp.GetSessionId())
	if !ok {
		r.sendToPlayer(from.ID, envelopeError(pb.ErrorCode_ERROR_CODE_UNKNOWN_SESSION, "session no longer pending"))
		return
	}

	from.SetLinkSessionID(session.id)
	if initiator := r.players.ByID(session.initiatorID); initiator != nil {
		initiator.SetLinkSessionID(session.id)
	}

	confirmation := &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_LinkSessionResponse{LinkSessionResponse: &pb.LinkSessionResponse{
			SessionId:         session.id,
			InitiatorPlayerId: uint32(session.initiatorID),
			TargetPlayerId:    uint32(session.targetID),
			Mode:              session.mode,
			Accepted:          true,
		}},
	}
	r.sendToPlayer(session.initiatorID, confirmation)
	r.sendToPlayer(session.targetID, confirmation)
}

func (r *Router) handleLinkSessionData(from *player.Session, data *pb.LinkSessionData) {
	session, ok := r.links.Get(data.GetSessionId())
	if !ok {
		return // session ended/unknown; silently drop rather than error-spam
	}

	var recipient player.ID
	if session.initiatorID == from.ID {
		recipient = session.targetID
	} else if session.targetID == from.ID {
		recipient = session.initiatorID
	} else {
		return // from isn't a participant of this session; ignore
	}

	data.FromPlayerId = uint32(from.ID)
	r.sendToPlayer(recipient, &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload:         &pb.Envelope_LinkSessionData{LinkSessionData: data},
	})
}

func (r *Router) handleLinkSessionEnd(from *player.Session, end *pb.LinkSessionEnd) {
	session, ok := r.links.Get(end.GetSessionId())
	if !ok {
		return
	}
	r.links.End(end.GetSessionId())

	other := otherParty(session, from.ID)
	r.sendToPlayer(other, &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_LinkSessionEnd{LinkSessionEnd: &pb.LinkSessionEnd{
			SessionId: end.GetSessionId(),
			Reason:    end.GetReason(),
		}},
	})
}

// --- Broadcast / send helpers -----------------------------------------------

func (r *Router) sendToPlayer(id player.ID, env *pb.Envelope) {
	session := r.players.ByID(id)
	if session == nil {
		return
	}
	if c := r.connMgr.Get(session.ConnID); c != nil {
		c.Send(env)
	}
}

func (r *Router) broadcastExcept(exclude player.ID, env *pb.Envelope) {
	for _, session := range r.players.All() {
		if session.ID == exclude {
			continue
		}
		if c := r.connMgr.Get(session.ConnID); c != nil {
			c.Send(env)
		}
	}
}

func (r *Router) sendWorldSnapshot(session *player.Session, c *connection.Conn) {
	var infos []*pb.PlayerInfo
	for _, other := range r.players.All() {
		if other.ID == session.ID {
			continue
		}
		infos = append(infos, other.ToPlayerInfo())
	}
	c.Send(&pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload: &pb.Envelope_WorldSnapshot{WorldSnapshot: &pb.WorldSnapshot{
			Players: infos,
		}},
	})
}

// SendPing sends a liveness Ping to session. Called from HeartbeatMonitor.
func (r *Router) SendPing(session *player.Session) {
	r.sendToPlayer(session.ID, &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload:         &pb.Envelope_Ping{Ping: &pb.Ping{Nonce: uint64(time.Now().UnixNano())}},
	})
}

// --- small helpers -----------------------------------------------------

func envelopePlayerJoined(info *pb.PlayerInfo) *pb.Envelope {
	return &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload:         &pb.Envelope_PlayerJoined{PlayerJoined: &pb.PlayerJoined{Player: info}},
	}
}

func envelopeError(code pb.ErrorCode, message string) *pb.Envelope {
	return &pb.Envelope{
		ProtocolVersion: config.ProtocolVersion,
		Payload:         &pb.Envelope_Error{Error: &pb.ErrorMessage{Code: code, Message: message}},
	}
}

func toPBAuthResult(r auth.Result) pb.AuthResult {
	switch r {
	case auth.ResultOK:
		return pb.AuthResult_AUTH_RESULT_OK
	case auth.ResultUsernameTaken:
		return pb.AuthResult_AUTH_RESULT_USERNAME_TAKEN
	case auth.ResultInvalidName:
		return pb.AuthResult_AUTH_RESULT_INVALID_NAME
	case auth.ResultServerFull:
		return pb.AuthResult_AUTH_RESULT_SERVER_FULL
	default:
		return pb.AuthResult_AUTH_RESULT_UNSPECIFIED
	}
}
