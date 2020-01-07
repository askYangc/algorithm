package interf

//用于收包和发包，Read返回一个完整的报文。(只关注解决收包和粘包问题)
//Write用于发送一个完整的报文。
//c interface{} 实际上是conn Conn，具体到tcp就是tcp.TCPConn,udp就是udp.UDPConn
type TransReceiver interface {
	Read(c interface{}) ([]byte, error)
	Write(c interface{}, args ...[]byte) error
}
