package network

import "fmt"

var (
	errProtocolMismatch = fmt.Errorf("network: protocol version mismatch")
	errAuthFailed       = fmt.Errorf("network: authentication failed")
)

func errTimeout(stage string) error {
	return fmt.Errorf("network: %s timed out", stage)
}
