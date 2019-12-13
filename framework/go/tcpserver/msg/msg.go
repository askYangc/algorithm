package msg

import (
	"leaf/network/processor/binary"
	"tcpserver/login"
)

var Processor = binary.NewProcessor()

func init() {
	Processor.Register(242, login.ChanRPC, login.Dohello)
}
