package main

import (
	"fmt"
	"leaf/chanrpc"
)

func test(a []interface{}) (interface{}) {
	fmt.Println("in test")
	fmt.Println(a)
	return 5
}

func run_server(server *chanrpc.Server) {
	for {
		select {
			case ci := <- server.ChanCall:
				server.Exec(ci)
		}
	}
}

func cb(a interface{}, err error){
	fmt.Println("in cb")
	fmt.Println(a)
	if err != nil {
		fmt.Println(err)
	}
}

func main(){
	server := chanrpc.NewServer(10)
	server.Register("test", test)

	go run_server(server)

	client := chanrpc.NewClient(1)
	client.Attach(server)
	client.AsynCall("test", 1, cb)

	client.Cb(<-client.ChanAsynRet)

}