package binary

import (
	"fmt"
	"leaf/chanrpc"
	"leaf/log"
	"leaf/network/parser"
)

type Processor struct {
	router map[uint16]*MsgInfo
}

type MsgInfo struct {
	msgRouter     *chanrpc.Server
}

type BinaryMsg struct {
	B []byte
	Cmd uint16
}

func NewProcessor() *Processor {
	p := new(Processor)
	p.router = make(map[uint16]*MsgInfo)
	return p
}

func (p *Processor) Register(cmd uint16, server *chanrpc.Server, receiver interface{}) {
	if _, ok := p.router[cmd]; ok {
		log.Fatal("cmd %v is already is registered", cmd)
	}

	msg := new(MsgInfo)
	msg.msgRouter = server
	p.router[cmd] = msg

	server.Register(cmd, receiver)
	return
}

func (p *Processor) Route(m interface{}, userData interface{}) error {
	msg := m.(*BinaryMsg)

	dst, ok := p.router[msg.Cmd]
	if !ok {
		log.Fatal("cmd %hu is not register")
		return fmt.Errorf("cmd %v not registered", msg.Cmd)
	}

	if dst.msgRouter != nil {
		dst.msgRouter.Go(msg.Cmd, msg.B, userData)
	}
	return nil
}

func (p *Processor) Unmarshal(data []byte) (interface{}, error) {
	tcph := &parser.Tcph{}
	tcph.Unpack(data)

	msg := new(BinaryMsg)
	msg.Cmd = tcph.Command

	msg.B = data

	return msg, nil
}

func (p *Processor) Marshal(msg interface{}) ([][]byte, error) {
	s := make([][]byte, 1)
	m := msg.([]byte)
	s[0] = m
	return s, nil
}
