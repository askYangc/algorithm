package cluster

import (
	"leaf/conf"
	"leaf/network/tcp"
	"leaf/network/interf"
	"math"
	"time"
)

var (
	server  *tcp.TCPServer
	clients []*tcp.TCPClient
)

func Init() {
	if conf.ListenAddr != "" {
		server = new(tcp.TCPServer)
		server.Addr = conf.ListenAddr
		server.MaxConnNum = int(math.MaxInt32)
		server.PendingWriteNum = conf.PendingWriteNum
		server.NewConn = newConn

		server.Start()
	}

	for _, addr := range conf.ConnAddrs {
		client := new(tcp.TCPClient)
		client.Addr = addr
		client.ConnNum = 1
		client.ConnectInterval = 3 * time.Second
		client.PendingWriteNum = conf.PendingWriteNum
		client.NewConn = newConn

		client.Start()
		clients = append(clients, client)
	}
}

func Destroy() {
	if server != nil {
		server.Close()
	}

	for _, client := range clients {
		client.Close()
	}
}

type Agent struct {
	conn *tcp.TCPConn
}

func newConn(conn *tcp.TCPConn) interf.ConnAgent {
	a := new(Agent)
	a.conn = conn
	return a
}

func (a *Agent) Run() {}

func (a *Agent) OnClose() {}
