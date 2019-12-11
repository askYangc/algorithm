package chanrpc

import (
	"fmt"
	"sync"
	"testing"
)

/*
Fail: 标记失败，但继续执行当前测试函数
FailNow: 失败，立即终止当前测试函数执行
Log: 输出错误信息
Error: Fail + Log
Fatal: FailNow + Log
Skip: 跳过当前函数，通常用于未完成的测试用例

t.Fail()
t.Log()

go test -v 才会显示t.log的数据，显示细节
go test -run regex 指定执行的函数名字

*/

func test(a []interface{}) (interface{}) {
	return 5
}

func run_server(server *Server) {
	for {
		select {
			case ci := <- server.ChanCall:
				server.Exec(ci)
		}
	}
}

func cb(a interface{}, err error){
	fmt.Println("in cb")
	fmt.Println("get test result", a)
	if err != nil {
		fmt.Println(err)
	}
}

func TestAsyncCall(t *testing.T) {
	server := NewServer(10)
	server.Register("test", test)

	go run_server(server)

	client := NewClient(1)
	client.Attach(server)
	client.AsynCall("test", 1, cb)

	client.Cb(<-client.ChanAsynRet)

}

func TestExample(t *testing.T) {
	s := NewServer(10)

	var wg sync.WaitGroup
	wg.Add(1)

	// goroutine 1
	go func() {
		s.Register("f0", func(args []interface{}) {

		})

		s.Register("f1", func(args []interface{}) interface{} {
			return 1
		})

		s.Register("fn", func(args []interface{}) []interface{} {
			return []interface{}{1, 2, 3}
		})

		s.Register("add", func(args []interface{}) interface{} {
			n1 := args[0].(int)
			n2 := args[1].(int)
			return n1 + n2
		})

		wg.Done()

		for {
			s.Exec(<-s.ChanCall)
		}
	}()

	wg.Wait()
	wg.Add(1)

	// goroutine 2
	go func() {
		c := s.Open(10)

		// sync
		err := c.Call0("f0")
		if err != nil {
			fmt.Println(err)
		}

		r1, err := c.Call1("f1")
		if err != nil {
			fmt.Println(err)
		} else {
			fmt.Println(r1)
		}

		rn, err := c.CallN("fn")
		if err != nil {
			fmt.Println(err)
		} else {
			fmt.Println(rn[0], rn[1], rn[2])
		}

		ra, err := c.Call1("add", 1, 2)
		if err != nil {
			fmt.Println(err)
		} else {
			fmt.Println(ra)
		}

		// asyn
		c.AsynCall("f0", func(err error) {
			if err != nil {
				fmt.Println(err)
			}
		})

		c.AsynCall("f1", func(ret interface{}, err error) {
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(ret)
			}
		})

		c.AsynCall("fn", func(ret []interface{}, err error) {
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(ret[0], ret[1], ret[2])
			}
		})

		c.AsynCall("add", 1, 2, func(ret interface{}, err error) {
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(ret)
			}
		})

		c.Cb(<-c.ChanAsynRet)
		c.Cb(<-c.ChanAsynRet)
		c.Cb(<-c.ChanAsynRet)
		c.Cb(<-c.ChanAsynRet)

		// go
		s.Go("f0")

		wg.Done()
	}()

	wg.Wait()

	// Output:
	// 1
	// 1 2 3
	// 3
	// 1
	// 1 2 3
	// 3
}