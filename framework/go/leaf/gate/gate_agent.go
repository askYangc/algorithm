package gate

import "net"

/* 用于应用层调用使用的user_data */
type GateAgent interface {
	WriteMsg(msg interface{})
	WriteMsgReliable(msg interface{})
	LocalAddr() net.Addr
	RemoteAddr() net.Addr
	Close()
	Destroy()
	UserData() interface{}
	SetUserData(data interface{})
}
