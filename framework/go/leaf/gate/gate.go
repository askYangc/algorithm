package gate

import (
	"leaf/chanrpc"
	"leaf/log"
	"leaf/network/interf"
	"leaf/network/tcp"
	"leaf/network/udp"
	"net"
	"reflect"
)

type Gate struct {
	MaxConnNum      int
	PendingWriteNum int
	Processor       interf.Processor
	AgentChanRPC    *chanrpc.Server

	// tcp
	TCPAddr      string
	TransReceiver    interf.TransReceiver

	//udp
	UDPAddr      string
	Sessioner interf.UDPSessioner
}

func (gate *Gate) Run(closeSig chan bool) {
	var tcpServer *tcp.TCPServer
	var udpServer *udp.UDPServer
	if gate.TCPAddr != "" {
		tcpServer = new(tcp.TCPServer)
		tcpServer.Addr = gate.TCPAddr
		tcpServer.MaxConnNum = gate.MaxConnNum
		tcpServer.PendingWriteNum = gate.PendingWriteNum
		tcpServer.TransReceiver = gate.TransReceiver

		tcpServer.NewConn = func(conn *tcp.TCPConn) interf.ConnAgent {
			a := &agent{conn: conn, gate: gate}
			if gate.AgentChanRPC != nil {
				gate.AgentChanRPC.Go("NewAgent", a)
			}
			return a
		}
	}

	if gate.UDPAddr != "" {
		udpServer = new(udp.UDPServer)
		udpServer.Addr = gate.UDPAddr
		udpServer.PendingWriteNum = gate.PendingWriteNum
		udpServer.Sessionser = gate.Sessioner
		udpServer.Sessionser.SetServer(udpServer)

		udpServer.NewConn = func(conn interf.Conn) interf.ConnAgent {
			a := &agent{conn: conn, gate: gate}
			if gate.AgentChanRPC != nil {
				gate.AgentChanRPC.Go("NewAgent", a)
			}
			return a
		}
	}

	if tcpServer != nil {
		tcpServer.Start()
	}

	if udpServer != nil {
		udpServer.Start()
	}

	<-closeSig

	if tcpServer != nil {
		tcpServer.Close()
	}
	if udpServer != nil {
		udpServer.Close()
	}

}

func (gate *Gate) OnDestroy() {}

type agent struct {
	conn     interf.Conn
	gate     *Gate
	userData interface{}
}

func (a *agent) Run() {
	for {
		data, err := a.conn.ReadMsg()
		if err != nil {
			log.Debug("read message: %v", err)
			break
		}

		if a.gate.Processor != nil {
			msg, err := a.gate.Processor.Unmarshal(data)
			if err != nil {
				log.Debug("unmarshal message error: %v", err)
				break
			}
			err = a.gate.Processor.Route(msg, a)
			if err != nil {
				log.Debug("route message error: %v", err)
				break
			}
		}
	}
}

func (a *agent) OnClose() {
	if a.gate.AgentChanRPC != nil {
		err := a.gate.AgentChanRPC.Call0("CloseAgent", a)
		if err != nil {
			log.Error("chanrpc error: %v", err)
		}
	}
}

func (a *agent) WriteMsg(flag interf.Flags, msg interface{}) {
	if a.gate.Processor != nil {
		data, err := a.gate.Processor.Marshal(msg)
		if err != nil {
			log.Error("marshal message %v error: %v", reflect.TypeOf(msg), err)
			return
		}
		err = a.conn.WriteMsg(flag, data...)
		if err != nil {
			log.Error("write message %v error: %v", reflect.TypeOf(msg), err)
		}
	}
}

func (a *agent) LocalAddr() net.Addr {
	return a.conn.LocalAddr()
}

func (a *agent) RemoteAddr() net.Addr {
	return a.conn.RemoteAddr()
}

func (a *agent) Close() {
	a.conn.Close()
}

func (a *agent) Destroy() {
	a.conn.Destroy()
}

func (a *agent) UserData() interface{} {
	return a.userData
}

func (a *agent) SetUserData(data interface{}) {
	a.userData = data
}
