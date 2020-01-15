package session

import (
	"fmt"
	"leaf/network/interf"
	"leaf/network/receiver"
	"leaf/network/udp"
	"leaf/util"
	"time"
)

type ClientSession struct {
	udp.UDPConn

	rand1 [4]byte
	cid uint32
	hashKey string

	user *ClientUser

	cond		util.SingleCond
	SendQueue 	*util.Queue
	sendRetry 	int

	myRequestId uint8
	peerRequestId uint8

	tDie	*time.Timer
}

func (s *ClientSession) UpdateDieTimer(t time.Duration) {
	if s.tDie == nil {
		s.tDie = time.NewTimer(t)
	}else {
		s.tDie.Reset(t)
	}
}

func (s *ClientSession) RequestAdd(ucph *receiver.Ucph, args ...[]byte) error {
	for i := 0; i < len(args); i++ {
		ucph.Param_len = uint16(len(args[i]))
		ucph.Request_id = s.myRequestId
		s.myRequestId++
		hdr, _ := ucph.Pack()
		s.SendQueue.Push(util.ByteJoin(hdr, args[i]))
	}

	s.cond.Notify()

	return nil
}

func (s *ClientSession) RequestDel()  {
	s.SendQueue.Pop()
	s.Lock()
	s.sendRetry = 0
	s.Unlock()
	return
}

func (s *ClientSession) CheckRequestId(ucph receiver.Ucph) (bool) {
	if ucph.Request_id == s.peerRequestId {
		return true
	}else if ucph.Request_id == (s.peerRequestId+1) {
		fmt.Printf("update rquest id to %u\n", ucph.Request_id)
		s.peerRequestId++
		return true
	}

	return false
}

func (s *ClientSession) CheckReplyId(reply receiver.Ucph) (bool) {
	if s.SendQueue.IsEmpty() {
		fmt.Printf("ignore reply pkt: no request in send list.\n")
		return false
	}

	if msg, ok := s.SendQueue.GetFront();ok {
		ucph := receiver.Ucph{}
		ucph.Unpack(msg.([]byte))
		if reply.Request_id != ucph.Request_id {
			fmt.Printf("%s ignore reply pkt: my request id=%u, but %u in pkt.\n", ucph.Request_id, reply.Request_id)
			return false
		}
		return true
	}

	fmt.Printf("ignore reply pkt2: no request in send list.\n")
	return false
}

func (s *ClientSession) ReadMsg() ([]byte, error) {
	for {
		msg, err := s.UDPConn.ReadMsg()
		if err != nil {
			return msg, err
		}

		ucph := receiver.Ucph{}
		_, err = ucph.Unpack(msg)
		if err != nil {
			continue
		}

		if ucph.Command == 242 {
			return msg,nil
		}

		if ucph.Request == 1 {
			//check reliable
			if ok := s.CheckRequestId(ucph);!ok {
				continue
			}
		}else {
			//check reply
			if ok := s.CheckReplyId(ucph);!ok {
				continue
			}
			s.RequestDel()
		}

		s.UpdateDieTimer(time.Second*10)

		return msg, nil
	}
}

func (s *ClientSession) WriteMsg(f interf.Flags, args ...[]byte) error {
	var is_request uint8 = 0
	if f.IsRequest {
		is_request = 1
	}
	ucph := receiver.NewUcph(uint16(f.Cmd), 0, is_request, 0, f.Flags)
	ucph.Client_sid = s.cid
	ucph.Request_id = f.ReqId

	if f.IsRequest && f.IsReliable {
		return s.RequestAdd(ucph, args...)
	}

	newArgs := [][]byte{}
	for i := 0; i < len(args); i++ {
		ucph.Param_len = uint16(len(args[i]))
		hdr, _ := ucph.Pack()
		b := util.ByteJoin(hdr, args[i])
		newArgs = append(newArgs, b)
	}

	return s.UDPConn.WriteMsgDirect(newArgs...)
}

func (s *ClientSession) Destory(sendReset bool) {
	if s.CloseFlag {
		return
	}
	if sendReset {
		//send reset
	}

	s.tDie.Stop()
	s.cond.Notify()
	s.UDPConn.Close()
}

func (s *ClientSession) Close() {
	s.Destory(true)
}