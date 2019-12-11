package internal

import (
	"leaf/gate"
	"reflect"
	"leaf/log"
	"leafserver/msg"
)

func handleMsg(m interface{}, h interface{}) {
	skeleton.RegisterChanRPC(reflect.TypeOf(m), h)
}

func dohello(args []interface{}) {
	m := args[0].(*msg.Hello)
	a := args[1].(gate.GateAgent)

	log.Debug("get msg: %v\n", m.Name)

	a.WriteMsg(&msg.Hello{
		Name:"client",
	})
}

func init() {
	handleMsg(&msg.Hello{}, dohello)
}
