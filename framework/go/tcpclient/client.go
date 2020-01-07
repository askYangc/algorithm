package main

import (
	"bytes"
	"fmt"
	"leaf/log"
	"leaf/network/interf"
	"leaf/network/processor/binary"
	"leaf/network/receiver"
	"leaf/network/tcp"
	"reflect"
	"time"
)

type agent struct{
	conn interf.Conn
	processor interf.Processor
}

func (a *agent) WriteMsg(msg interface{}) {
	if a.processor != nil {
		data, err := a.processor.Marshal(msg)
		if err != nil {
			log.Error("marshal message %v error: %v", reflect.TypeOf(msg), err)
			return
		}
		err = a.conn.WriteMsg(data...)
		if err != nil {
			log.Error("write message %v error: %v", reflect.TypeOf(msg), err)
		}
	}
}

func (a *agent) auth() {
	tcph := &receiver.Tcph{}

	param := []byte("hello leaf")

	tcph.Ver = 1
	tcph.Request = 1
	tcph.Hlen = 4
	tcph.Encrypt = 0
	tcph.Command = 242
	tcph.Param_len = uint32(len(param))

	b, err := tcph.Pack()
	if err != nil{
		fmt.Println("Pack failed")
	}

	var buffer bytes.Buffer
	buffer.Write(b)
	buffer.Write(param)
	total := buffer.Bytes()

	a.WriteMsg(total)
}

func getreply(u interface{}) {
	msg := u.(*binary.BinaryMsg)
	fmt.Printf("msg: %u\n", msg.Cmd)
	tcph := &receiver.Tcph{}
	offset, _ := tcph.Unpack(msg.B)
	fmt.Printf("%s\n", msg.B[offset:])
}

func (a *agent) Run() {
	fmt.Println("in run")
	a.auth()

	data, err := a.conn.ReadMsg()
	if err != nil {
		log.Debug("read message: %v", err)
		return
	}

	if a.processor != nil {
		m, err := a.processor.Unmarshal(data)
		if err != nil {
			log.Debug("unmarshal message error: %v", err)
			return
		}
		getreply(m)
	}

}

func (a *agent) OnClose() {

}

var Processor = binary.NewProcessor()

func main() {
	addr := "127.0.0.1:3563"
	//Processor.Register(242)
	client := &tcp.TCPClient{
		Addr: addr,
		ConnNum: 1,
		NewConn: func (conn *tcp.TCPConn) interf.ConnAgent{
			a := new(agent)
			a.conn = conn
			a.processor = Processor
			return a
		},
		TransReceiver: receiver.NewExampleTcpReceiver(),
	}

	client.Start()
	for {
		time.Sleep(5 * time.Second)
		break

	}
	fmt.Println("over")
}