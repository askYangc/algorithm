package main

import (
	"fmt"
	"reflect"
	"leaf/network/example"
	"leaf/network/example/json"
	"leaf/network/interf"
	"leaf/network/tcp"
	"leaf/log"
	"leafserver/msg"
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

func (a *agent) Run() {
	fmt.Println("in run")
	a.WriteMsg(&msg.Hello{
		Name:"leaf",
	})

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
		c := m.(*msg.Hello)
		fmt.Println(c.Name)
	}

}

func (a *agent) OnClose() {

}

var Processor = json.NewProcessor()

func main() {
	addr := "127.0.0.1:3563"
	Processor.Register(&msg.Hello{})
	client := &tcp.TCPClient{
		Addr: addr,
		ConnNum: 1,
		NewConn: func (conn *tcp.TCPConn) interf.ConnAgent{
			a := new(agent)
			a.conn = conn
			a.processor = Processor
			return a
		},
		MsgParser: example.NewExampleParser(),
	}

	client.Start()
	for {
		time.Sleep(5 * time.Second)
		break

	}
	fmt.Println("over")
}