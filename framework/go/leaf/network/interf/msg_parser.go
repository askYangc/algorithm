package interf

import "net"

//用于收包和发包，Read返回一个完整的报文。(只关注解决收包和粘包问题)
//Write用于发送一个完整的报文。
//c interface{} 实际上是conn Conn，具体到tcp就是tcp.TCPConn,udp就是udp.UDPConn
type MsgParser interface {
	Read(conn net.Conn) ([]byte, error)
	Write(conn net.Conn, args ...[]byte) error
}
