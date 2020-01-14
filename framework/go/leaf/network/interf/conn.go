package interf

import (
	"net"
)

type Flags struct {
	Cmd   uint32
	IsRequest bool
	IsReliable bool
	Flags uint8
}

type Conn interface {
	ReadMsg() ([]byte, error)
	WriteMsg(f Flags, args ...[]byte) error
	LocalAddr() net.Addr
	RemoteAddr() net.Addr
	Close()
	Destroy()
}