package internal

import (
	"leaf/gate"
	"leaf/network/parser"
	"tcpserver/conf"
	"tcpserver/msg"
)

type Module struct {
	*gate.Gate
}

func (m *Module) OnInit() {
	p := parser.NewTcphParser()
	m.Gate = &gate.Gate{
		MaxConnNum:      conf.Server.MaxConnNum,
		PendingWriteNum: conf.PendingWriteNum,
		TCPAddr:         conf.Server.TCPAddr,
		Processor:       msg.Processor,
		MsgParser : 	p,
		AgentChanRPC:    nil,
	}
}
