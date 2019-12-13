源代码地址为：https://github.com/name5566/leaf
我做了部分修改

将解析器Parser 独立出来做成了一个接口
	Read(conn net.Conn) ([]byte, error)
	Write(conn net.Conn, args ...[]byte) error

	Read用于接收完整的一片报文并返回
	Write用于将报文发送出去
	例子协议查看example_parser.go
	二进制协议查看tcph_parser.go

processor未作修改
	// must goroutine safe
	Route(msg interface{}, userData interface{}) error
	// must goroutine safe
	Unmarshal(data []byte) (interface{}, error)
	// must goroutine safe
	Marshal(msg interface{}) ([][]byte, error)

对于json或者protobuf,查看network/processor/json/json.go或者network/processor/protobuf/protobuf.go
Route用于将数据发送到指定模块。
Unmarshal用于解码，比如讲二进制报文转换为结构体。
Marshal用于将结构体转换为二进制报文。

对于具有可变长度的二进制协议,查看network/processor/binary/binary.go
Route用于将二进制数据发送到指定模块，由模块自己解析。
Unmarshal用于解码协议头部，知道具体的命令报文，以便Route进行转发。
Marshal 一般没什么用，只是单纯讲[]byte加入到[][]byte，以便parser.Write调用。

对于原版leaf，一个协议需要在Processor、gate、game三个地方注册，以hello{}为例
Processor注册用于将hello{}登记，一遍收到json时将二进制数据转换为Hello结构体
gate注册用于将hello{}转发给game.ChanRPC
game注册用于收到hello{}后转交给指定函数处理。

个人观点感觉太繁琐了。
所以设计了binary.go里面的一个函数
Register(cmd uint16, server *chanrpc.Server, receiver interface{})
这样注册的时候，将上面3个合成在一起了。
指定了该cmd转发给哪个模块。
同时给该模块注册了指定函数去处理。
需要Register在init中注册。不然可能导致协程出错。
当然也可以在main函数中第一个协程里面注册，避免协程安全问题