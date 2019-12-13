package login

import (
	"leaf/gate"
	"leaf/log"
	"leaf/network/parser"
	"leaf/util"
	"tcpserver/login/internal"
)

var (
	Module  = new(internal.Module)
	ChanRPC = internal.ChanRPC
)

func Dohello(args []interface{}) {
	m := args[0].([]byte)
	a := args[1].(gate.GateAgent)

	tcph := &parser.Tcph{}
	offset,_ := tcph.Unpack(m)
	log.Debug("data: %hhu, ver: %hhu, hlen: %hhu, enc: %hhu, req: %hhu, cmd: %hu, len:%u\n",
		m[0], tcph.Ver, tcph.Hlen, tcph.Encrypt, tcph.Request, tcph.Command, tcph.Param_len)
	log.Debug("get msg: %s\n", m[offset:])

	tcph.Request = 0

	reply := []byte("hello client")

	tcph.Param_len = uint32(len(reply))
	br, _ := tcph.Pack()
	br = util.ByteJoin(br, reply)

	a.WriteMsg(br)
}