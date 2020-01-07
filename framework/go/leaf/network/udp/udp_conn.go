package udp

import (
	"fmt"
	"net"
	"sync"
)

type UDPConn struct {
	sync.Mutex
	addr *net.UDPAddr
	server *UDPServer
	closeFlag bool

	RecvChan	chan []byte
}

func newUDPConn(server *UDPServer, pendingWriteNum int) *UDPConn {
	udpConn := new(UDPConn)

	udpConn.Init(server, pendingWriteNum)
	return udpConn
}

func (udpConn *UDPConn) Init(server *UDPServer, pendingWriteNum int) {
	udpConn.server = server
	udpConn.RecvChan = make(chan []byte, pendingWriteNum)
	udpConn.closeFlag = false
}

func (udpConn *UDPConn) destroy() {
	if !udpConn.closeFlag {
		close(udpConn.RecvChan)
		udpConn.closeFlag = true
	}
}

func (udpConn *UDPConn) Destroy() {
	udpConn.Lock()
	defer udpConn.Unlock()

	udpConn.destroy()
}

func (udpConn *UDPConn) Close() {
	udpConn.Destroy()
}

func (udpConn *UDPConn) LocalAddr() net.Addr {
	return udpConn.server.LocalAddr
}

func (udpConn *UDPConn) RemoteAddr() net.Addr {
	return udpConn.addr
}

//这里考虑加一个定时，以免RecvChan阻塞导致整个系统阻塞
func (udpConn *UDPConn) TransMsg(b []byte) (error) {
	udpConn.Lock()
	defer udpConn.Unlock()

	if udpConn.closeFlag || b == nil {
		return nil
	}

	udpConn.RecvChan <- b
	return nil
}

func (udpConn *UDPConn) ReadMsg() ([]byte, error) {
	if b, ok := <-udpConn.RecvChan; ok {
		return b, nil
	}
	return nil, fmt.Errorf("RecvChan maybe close")
}

func (udpConn *UDPConn) WriteMsg(args ...[]byte) error {
	for i := 0; i < len(args); i++ {
		msg := &UDPSendMsg{buf:args[i], addr:udpConn.addr}
		udpConn.server.SendList <- msg
	}
	return nil
}

func (udpConn *UDPConn) WriteMsgReliable(args ...[]byte) error {
	for i := 0; i < len(args); i++ {
		msg := &UDPSendMsg{buf:args[i], addr:udpConn.addr}
		udpConn.server.SendList <- msg
	}
	return nil
}

