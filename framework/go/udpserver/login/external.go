package login

import (
	"fmt"
	"leaf/gate"
	"leaf/log"
	"leaf/network/interf"
	"leaf/network/receiver"
	"udpserver/login/internal"
)

var (
	Module  = new(internal.Module)
	ChanRPC = internal.ChanRPC
)

func Dohello(args []interface{}) {
	m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	ucph := &receiver.Ucph{}
	offset,_ := ucph.Unpack(m)
	log.Debug("data: %hhu, ver: %hhu, hlen: %hhu, enc: %hhu, req: %hhu, request_id:%hhu,  cmd: %hu, len:%u\n",
		m[0], ucph.Ver, ucph.Hlen, ucph.Encrypt, ucph.Request, ucph.Request_id, ucph.Command, ucph.Param_len)

	ar := &receiver.ClientAuthRequest{}
	offset,_ = ar.Unpack(m[offset:])

	log.Debug("get action %d, ver %d username %s, rand1 %s\n", ar.Action, ar.Action, ar.Username, ar.Rand1)

	reply := []byte("hello client")

	fmt.Println(reply)
	fmt.Printf("data %s\n", reply)
	a.WriteMsg(interf.Flags{uint32(ucph.Command),false, false, 0, 0}, reply)
}

func DoReset(args []interface{}) {
	//m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	fmt.Println("get close")
	a.Close()

}

func DoKeeplive(args []interface{}) {
	m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	//fmt.Println("get keep")
	ucph := receiver.Ucph{}
	ucph.Unpack(m)

	a.WriteMsg(interf.Flags{uint32(ucph.Command),false, false, 0, ucph.Request_id}, []byte{})
	//a.Close()

}