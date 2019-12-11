package gate

import (
	"leafserver/login"
	"leafserver/msg"
)

func init() {
	msg.Processor.SetRouter(&msg.Hello{}, login.ChanRPC)
}
