package interf

import "net"

/*
udp报文处理函数
*/
type UDPSessioner interface {
	RouteToSession(buf []byte, addr *net.UDPAddr)
	SetServer(server interface{}) (bool)
	Stop()
}
