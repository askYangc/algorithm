package udp

import (
	"fmt"
	"leaf/log"
	"leaf/network/interf"
	"net"
	"sync"
)


type UDPSendMsg struct {
	buf []byte
	addr *net.UDPAddr
}

/*
1, 收包
2，找到会话并分配给指定会话。
3，会话拥有conn以便发包。
*/
type UDPServer struct {
	Conn 			*net.UDPConn

	Addr            string
	LocalAddr		*net.UDPAddr

	wgLn            sync.WaitGroup
	SendListCount	uint32
	PendingWriteNum int

	SendList		chan *UDPSendMsg

	//udp Session
	Sessions    interf.UDPSessioner
	NewConn     func(interf.Conn) interf.ConnAgent
}

func (server *UDPServer) Start() {
	server.init()
	go server.run()
}

func (server *UDPServer) init() {
	addr, err := net.ResolveUDPAddr("udp", server.Addr)
	if err != nil {
		log.Fatal("%v", err)
	}

	server.LocalAddr = addr
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		log.Fatal("%v", err)
	}

	if server.SendListCount == 0 {
		server.SendListCount = 10240
	}

	server.SendList = make(chan *UDPSendMsg, server.SendListCount)

	if server.Sessions == nil {
		server.Sessions = NewUDPSessionManager(server)
	}

	if server.PendingWriteNum == 0 {
		server.PendingWriteNum = 1024
	}

	server.Conn = conn

	go func() {
		for msg := range server.SendList {
			conn.WriteToUDP(msg.buf, msg.addr)
		}
		fmt.Printf("send list over\n")
	}()
}



func (server *UDPServer) run() {
	server.wgLn.Add(1)
	defer server.wgLn.Done()

	buf := make([]byte, 1500)

	for {
		n, addr, err := server.Conn.ReadFromUDP(buf)
		if err != nil {
			fmt.Println("server ReadFromUDP close")
			break
		}

		server.Sessions.RouteToSession(buf, n, addr)
	}
}


func (server *UDPServer) Close() {
	server.Conn.Close()
	server.wgLn.Wait()

	server.Sessions.Stop()
	server.Conn = nil
	server.LocalAddr = nil
	close(server.SendList)
}
