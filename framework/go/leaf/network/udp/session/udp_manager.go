package session

import (
	"fmt"
	"leaf/log"
	"leaf/network/receiver"
	"leaf/network/udp"
	"leaf/util"
	"net"
	"sync"
	"time"
)

//cid -> ClientSession
type SessionMap map[uint32]*ClientSession
//ip+port得到Key+username，然后遍历得到Session
type SessionHash map[string]*ClientSession
//username -> User
type SessionUser map[string]*ClientUser

type UDPSessionManager struct {
	server 		*udp.UDPServer

	cid 		uint32
	SessionCount int
	sessions	SessionMap
	sessionHash	SessionHash
	users 		SessionUser
	mutexConns      sync.Mutex
	wgConns         sync.WaitGroup
}

func NewUDPSessionManager() *UDPSessionManager {
	s := new(UDPSessionManager)
	s.SessionCount = 0
	s.cid = 1
	s.sessions = make(SessionMap)
	s.sessionHash = make(SessionHash)
	s.users = make(SessionUser)
	return s
}

func (s *UDPSessionManager) SetServer(server interface{}) (bool) {
	udpserver, ok := server.(*udp.UDPServer)
	if ok {
		s.server = udpserver
		return true
	}
	return false
}

func (s *UDPSessionManager) allocSessionId() (uint32, error){
	cid := s.cid
	s.cid++
	return cid,nil
}

func (s *UDPSessionManager) sessionHashKey(addr *net.UDPAddr, username[]byte) string{
	return util.StringJoin(addr.String(), string(username))
}

func (s *UDPSessionManager) sessionHashLookUp(addr *net.UDPAddr, username[]byte) (*ClientSession, error){
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	key := s.sessionHashKey(addr, username)
	if session, ok := s.sessionHash[key];ok {
		return session, nil
	}
	return nil, fmt.Errorf("not found key %s\n", key)
}

func (s *UDPSessionManager) sessionHashLookUpByCid(cid uint32) (*ClientSession, error){
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	if session, ok := s.sessions[cid];ok {
		return session, nil
	}
	return nil, fmt.Errorf("not found cid %u\n", cid)
}

func (s *UDPSessionManager) CreateSession(user *ClientUser, addr *net.UDPAddr, ar *receiver.ClientAuthRequest) (*ClientSession, error) {
	cid, err := s.allocSessionId()
	if err != nil {
		return nil, err
	}

	clientSession := new(ClientSession)
	clientSession.Addr = addr
	clientSession.hashKey = s.sessionHashKey(addr, ar.Username[:])
	clientSession.Init(s.server, s.server.PendingWriteNum)
	clientSession.SendQueue = util.NewQueue(true)
	clientSession.cond = util.MakeSingleCond()
	clientSession.cid = cid
	clientSession.user = user
	clientSession.m = s
	clientSession.myRequestId = 0
	clientSession.peerRequestId = 0

	user.auth_session = clientSession

	s.mutexConns.Lock()
	s.sessions[cid] = clientSession
	s.sessionHash[clientSession.hashKey] = clientSession
	s.SessionCount++
	s.mutexConns.Unlock()

	s.wgConns.Add(2)
	agent := s.server.NewConn(clientSession)

	fmt.Printf("now create session cid %u\n", cid)
	go func() {
		agent.Run()

		fmt.Println("Run over")
		s.mutexConns.Lock()
		s.SessionCount--
		s.deleteSession(clientSession)
		s.mutexConns.Unlock()

		agent.OnClose()
		s.wgConns.Done()
	}()

	go func() {
		delay := time.Second*3
		for !clientSession.CloseFlag{
			select {
			case <-time.After(delay):
				break
			case <- clientSession.cond.GetSignelCond():
				break
			}

			if clientSession.CloseFlag{
				break
			}

			if m, ok := clientSession.SendQueue.GetFront();ok {
				msg := m.([]byte)
				clientSession.WriteMsgDirect(msg)
			}else {
				delay = time.Second*3
				clientSession.sendRetry = 0
				continue
			}

			clientSession.Lock()
			if clientSession.sendRetry >= 3 {
				clientSession.sendRetry = 2
				delay = time.Millisecond *3000
			}else {
				delay = time.Millisecond*100
			}
			clientSession.sendRetry++
			clientSession.Unlock()
		}
		fmt.Println("send request Queue stop")
		s.wgConns.Done()
	}()

	return clientSession, nil
}

func (s *UDPSessionManager) deleteSession(session *ClientSession){
	delete(s.sessions, session.cid)
	delete(s.sessionHash, session.hashKey)
	return
}

func (s *UDPSessionManager) DeleteSession(session *ClientSession){
	s.mutexConns.Lock()
	s.deleteSession(session)
	s.mutexConns.Unlock()
	return
}

func (s *UDPSessionManager) CloseSession(cid uint32) (bool){
	session, err := s.sessionHashLookUpByCid(cid)
	if err != nil {
		return false
	}

	session.Close()
	s.DeleteSession(session)
	return true
}

func (s *UDPSessionManager) clientSessionLookUpByUsername(addr *net.UDPAddr, username []byte) (*ClientSession, error) {
	session, err := s.sessionHashLookUp(addr, username)
	if err != nil {
		return nil, err
	}

	return session, nil
}

func (s *UDPSessionManager) clientSessionLookUp(cid uint32) (*ClientSession, error) {
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	if session, ok := s.sessions[cid];ok {
		return session, nil
	}
	return nil, fmt.Errorf("not found cid %u\n", cid)
}

func (s *UDPSessionManager) clientUserLookUp(username []byte) (*ClientUser, error) {
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	key,_ := util.ByteToString(username, len(username))

	if user, ok := s.users[key];ok {
		return user, nil
	}

	user, err := NewClientUser(username)
	if err != nil {
		return nil, err
	}

	s.users[key] = user

	return user, nil
}

func (s *UDPSessionManager) clientTryUpdateIp(session *ClientSession, addr *net.UDPAddr){
	if session.Addr.String() != addr.String() {
		s.mutexConns.Lock()
		delete(s.sessionHash, session.hashKey)
		session.hashKey = s.sessionHashKey(addr, session.user.username[:])
		s.sessionHash[session.hashKey] = session
		session.Addr = addr
		s.mutexConns.Unlock()
	}
}

func (s *UDPSessionManager) ClientSessionGet(buf []byte, addr *net.UDPAddr) (*ClientSession, error) {
	ucph := &receiver.Ucph{}
	ar := &receiver.ClientAuthRequest{}
	n := len(buf)

	if uint32(len(buf)) < ucph.GetLen() {
		return nil, fmt.Errorf("len %d is too short\n", len(buf))
	}

	offset,_ := ucph.Unpack(buf)
	cid := ucph.Client_sid

	if cid == 0 {
		if ucph.Encrypt != 0 || ucph.Command != 242 {
			log.Error("no client session id, but cmd=%u, enc=%d\n", ucph.Command, ucph.Encrypt)
			return nil, fmt.Errorf("no device session id, but cmd=%u, enc=%d\n", ucph.Command, ucph.Encrypt)
		}

		if uint32(ucph.Param_len) < ar.GetLen() || n < int(uint32(ucph.Hlen) + ar.GetLen()){
			log.Error("bad pkt len, len=%u, param_len=%u\n", n, ucph.Param_len)
			return nil, fmt.Errorf("bad pkt len, len=%u, param_len=%u\n", n, ucph.Param_len)
		}

		ar.Unpack(buf[offset:])
		if ar.Action != 3 {
			log.Error("no client session id, but action=%u\n", ar.Action)
			return nil, fmt.Errorf("no client session id, but action=%u\n", ar.Action)
		}

		session, err := s.clientSessionLookUpByUsername(addr, ar.Username[:])
		if session != nil && err == nil {
			log.Debug("client_session_lookup: ok, ip,port:%s\n", addr.String())
			util.ByteCopy(session.rand1[:], ar.Rand1[:], len(ar.Rand1))
			return session, nil
		}

		user, err := s.clientUserLookUp(ar.Username[:])
		if err != nil {
			return nil, fmt.Errorf("User %s not found\n", ar.Username)
		}

		if(user.estab_session != nil && user.estab_session.Addr.String() == addr.String()) {
			log.Debug("esta_session is not NULL, s->cid:%u\n", user.estab_session.cid);
			return user.estab_session, nil
		}

		if(user.auth_session != nil) {
			if user.auth_session.Addr.String() == addr.String() {
				log.Debug("auth_session ip:%s,sock:%d, s->cid:%d\n", addr.String(), user.auth_session.cid);
				util.ByteCopy(user.auth_session.rand1[:], ar.Rand1[:], len(ar.Rand1))
				return user.auth_session, nil
			}else {
				log.Debug("usersession:0x%08X have new device connect(%s), delete old one(%s)\n",
					user.auth_session.cid, addr.String(), user.auth_session.Addr.String());
				user.auth_session.Destory(true)
			}
		}

		user.auth_session, _ = s.CreateSession(user, addr, ar)
		return user.auth_session, nil
	}

	session, err := s.clientSessionLookUp(cid)
	if err != nil {
		return nil, fmt.Errorf("Cid %u not found\n", cid)
	}

	s.clientTryUpdateIp(session, addr)

	return session, nil
}

/*
1，查找回话，检查报文合法性
2，转发报文到指定会话
*/
func (s *UDPSessionManager) RouteToSession(buf []byte, addr *net.UDPAddr) {
	session, err := s.ClientSessionGet(buf[:], addr)
	if err != nil {
		return
	}
	s.routeMsgToSession(session, buf)
}

func (s *UDPSessionManager) routeMsgToSession(session *ClientSession, buf []byte) {
	b := make([]byte, len(buf))
	util.ByteCopy(b, buf, len(buf))
	session.TransMsg(b)
}

func (s *UDPSessionManager) Stop() {
	for _, v := range s.sessions {
		v.Close()
	}

	s.mutexConns.Lock()
	s.SessionCount = 0
	s.cid = 1
	s.sessions = make(SessionMap)
	s.sessionHash = make(SessionHash)
	s.users = make(SessionUser)
	s.mutexConns.Unlock()

	s.wgConns.Wait()
}
