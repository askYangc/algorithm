package tcp

import (
	"leaf/log"
	"net"
	"leaf/network/interf"
	"sync"
)

type TCPConn struct {
	sync.Mutex
	Conn      net.Conn
	writeChan chan []byte
	closeFlag bool
	transReceiver interf.TransReceiver
}

func newTCPConn(conn net.Conn, pendingWriteNum int, transReceiver interf.TransReceiver) *TCPConn {
	tcpConn := new(TCPConn)
	tcpConn.Conn = conn
	tcpConn.writeChan = make(chan []byte, pendingWriteNum)
	tcpConn.transReceiver = transReceiver

	go func() {
		for b := range tcpConn.writeChan {
			if b == nil {
				break
			}

			_, err := conn.Write(b)
			if err != nil {
				break
			}
		}

		conn.Close()
		tcpConn.Lock()
		tcpConn.closeFlag = true
		tcpConn.Unlock()
	}()

	return tcpConn
}

func (tcpConn *TCPConn) doDestroy() {
	tcpConn.Conn.(*net.TCPConn).SetLinger(0)
	tcpConn.Conn.Close()

	if !tcpConn.closeFlag {
		close(tcpConn.writeChan)
		tcpConn.closeFlag = true
	}
}

func (tcpConn *TCPConn) Destroy() {
	tcpConn.Lock()
	defer tcpConn.Unlock()

	tcpConn.doDestroy()
}

func (tcpConn *TCPConn) Close() {
	tcpConn.Lock()
	defer tcpConn.Unlock()
	if tcpConn.closeFlag {
		return
	}

	tcpConn.doWrite(nil)
	tcpConn.closeFlag = true
}

func (tcpConn *TCPConn) doWrite(b []byte) {
	if len(tcpConn.writeChan) == cap(tcpConn.writeChan) {
		log.Debug("close conn: channel full")
		tcpConn.doDestroy()
		return
	}

	tcpConn.writeChan <- b
}

// b must not be modified by the others goroutines
func (tcpConn *TCPConn) Write(b []byte) {
	tcpConn.Lock()
	defer tcpConn.Unlock()
	if tcpConn.closeFlag || b == nil {
		return
	}

	tcpConn.doWrite(b)
}

func (tcpConn *TCPConn) Read(b []byte) (int, error) {
	return tcpConn.Conn.Read(b)
}

func (tcpConn *TCPConn) LocalAddr() net.Addr {
	return tcpConn.Conn.LocalAddr()
}

func (tcpConn *TCPConn) RemoteAddr() net.Addr {
	return tcpConn.Conn.RemoteAddr()
}

func (tcpConn *TCPConn) ReadMsg() ([]byte, error) {
	return tcpConn.transReceiver.Read(tcpConn)
}

func (tcpConn *TCPConn) WriteMsg(args ...[]byte) error {
	return tcpConn.transReceiver.Write(tcpConn, args...)
}

func (tcpConn *TCPConn) WriteMsgReliable(args ...[]byte) error {
	return tcpConn.transReceiver.Write(tcpConn, args...)
}
