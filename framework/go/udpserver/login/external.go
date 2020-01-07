package login

import (
	"fmt"
	"leaf/gate"
	"leaf/log"
	"leaf/network/receiver"
	"leaf/util"
	"udpserver/login/internal"
)

var (
	Module  = new(internal.Module)
	ChanRPC = internal.ChanRPC
)

func Dohello(args []interface{}) {
	m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	tcph := &receiver.Ucph{}
	offset,_ := tcph.Unpack(m)
	log.Debug("data: %hhu, ver: %hhu, hlen: %hhu, enc: %hhu, req: %hhu, request_id:%hhu,  cmd: %hu, len:%u\n",
		m[0], tcph.Ver, tcph.Hlen, tcph.Encrypt, tcph.Request, tcph.Request_id, tcph.Command, tcph.Param_len)

	ar := &receiver.ClientAuthRequest{}
	offset,_ = ar.Unpack(m[offset:])

	log.Debug("get action %d, ver %d username %s, rand1 %s\n", ar.Action, ar.Action, ar.Username, ar.Rand1)

	tcph.Request = 0
	tcph.Client_sid = 1

	reply := []byte("hello client")

	tcph.Param_len = uint16(len(reply))
	br, _ := tcph.Pack()
	br = util.ByteJoin(br, reply)
	fmt.Println(br)
	fmt.Printf("data %s\n", br)
	a.WriteMsg(br)
}

func DoReset(args []interface{}) {
	//m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	fmt.Println("get close")
	a.Close()
}