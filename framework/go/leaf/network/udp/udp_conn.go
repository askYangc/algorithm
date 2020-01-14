package udp

import (
	"fmt"
	"leaf/network/interf"
	"net"
	"sync"
)

type UDPConn struct {
	sync.Mutex
	Addr *net.UDPAddr
	Server *UDPServer
	CloseFlag bool

	RecvChan	chan []byte
}

func newUDPConn(server *UDPServer, pendingWriteNum int) *UDPConn {
	udpConn := new(UDPConn)

	udpConn.Init(server, pendingWriteNum)
	return udpConn
}

func (udpConn *UDPConn) Init(server *UDPServer, pendingWriteNum int) {
	udpConn.Server = server
	udpConn.RecvChan = make(chan []byte, pendingWriteNum)
	udpConn.CloseFlag = false
}

func (udpConn *UDPConn) destroy() {
	if !udpConn.CloseFlag {
		udpConn.CloseFlag = true
		close(udpConn.RecvChan)
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
	return udpConn.Server.LocalAddr
}

func (udpConn *UDPConn) RemoteAddr() net.Addr {
	return udpConn.Addr
}

//这里考虑加一个定时，以免RecvChan阻塞导致整个系统阻塞
func (udpConn *UDPConn) TransMsg(b []byte) (error) {
	udpConn.Lock()
	defer udpConn.Unlock()

	if udpConn.CloseFlag || b == nil {
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


func (udpConn *UDPConn) WriteMsg(f interf.Flags, args ...[]byte) error {
	return udpConn.WriteMsgDirect(args...)
}

func (udpConn *UDPConn) WriteMsgDirect(args ...[]byte) error {
	for i := 0; i < len(args); i++ {
		msg := &UDPSendMsg{buf:args[i], addr:udpConn.Addr}
		udpConn.Server.SendList <- msg
	}
	return nil
}


