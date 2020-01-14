package msg

import (
	"leaf/network/processor/binary"
	"udpserver/login"
)

var Processor = binary.NewProcessor()

func init() {
	Processor.Register(242, login.ChanRPC, login.Dohello)
	Processor.Register(243, login.ChanRPC, login.DoReset)
	Processor.Register(244, login.ChanRPC, login.DoKeeplive)
}
