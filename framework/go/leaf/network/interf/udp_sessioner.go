package interf

import "net"

/*
udp报文处理函数
*/
type UDPSessioner interface {
	RouteToSession(buf []byte, n int, addr *net.UDPAddr)
	Stop()
}
