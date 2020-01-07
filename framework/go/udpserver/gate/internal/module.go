package internal

import (
	"leaf/gate"
	"leaf/network/receiver"
	"udpserver/conf"
	"udpserver/msg"
)

type Module struct {
	*gate.Gate
}

func (m *Module) OnInit() {
	p := receiver.NewTCPTcphReceiver()
	m.Gate = &gate.Gate{
		MaxConnNum:      conf.Server.MaxConnNum,
		PendingWriteNum: conf.PendingWriteNum,
		UDPAddr:         conf.Server.TCPAddr,
		Processor:       msg.Processor,
		TransReceiver : 	p,
		AgentChanRPC:    nil,
	}
}
