package gate

import (
	"leaf/network/interf"
	"net"
)

/* 用于应用层调用使用的user_data */
type GateAgent interface {
	WriteMsg(flag interf.Flags, msg interface{})
	LocalAddr() net.Addr
	RemoteAddr() net.Addr
	Close()
	Destroy()
	UserData() interface{}
	SetUserData(data interface{})
}
