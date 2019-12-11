package internal

import (
	"leaf/gate"
	"leaf/network/example"
	"leafserver/conf"
	"leafserver/msg"
)

type Module struct {
	*gate.Gate
}

func (m *Module) OnInit() {
	p := example.NewExampleParser()
	m.Gate = &gate.Gate{
		MaxConnNum:      conf.Server.MaxConnNum,
		PendingWriteNum: conf.PendingWriteNum,
		TCPAddr:         conf.Server.TCPAddr,
		Processor:       msg.Processor,
		MsgParser : 	p,
		AgentChanRPC:    nil,
	}
}
