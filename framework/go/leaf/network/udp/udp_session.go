package udp

import (
	"fmt"
	"leaf/log"
	"leaf/network/receiver"
	"leaf/util"
	"net"
	"sync"
)

//cid -> ClientSession
type SessionMap map[uint32]*ClientSession
//ip+port得到Key+username，然后遍历得到Session
type SessionHash map[string]*ClientSession
//username -> User
type SessionUser map[string]*ClientUser

type UDPSessionManager struct {
	server 		*UDPServer

	cid 		uint32
	SessionCount int
	sessions	SessionMap
	sessionHash	SessionHash
	users 		SessionUser
	mutexConns      sync.Mutex
	wgConns         sync.WaitGroup
}

type ClientSession struct {
	UDPConn

	rand1 [4]byte
	cid uint32
	hashKey string

	user *ClientUser
}

type ClientUser struct {
	username [16]byte

	auth_session *ClientSession
	estab_session *ClientSession
}

func NewUDPSessionManager(server *UDPServer) *UDPSessionManager {
	s := new(UDPSessionManager)
	s.server = server
	s.SessionCount = 0
	s.cid = 1
	s.sessions = make(SessionMap)
	s.sessionHash = make(SessionHash)
	s.users = make(SessionUser)
	return s
}

func (s *UDPSessionManager) allocSessionId() (uint32, error){
	cid := s.cid
	s.cid++
	return cid,nil
}

func (s *UDPSessionManager) sessionHashKey(addr *net.UDPAddr, username[]byte, n int) string{
	return util.StringJoin(addr.String(), string(username[:n]))
}

func (s *UDPSessionManager) sessionHashLookUp(addr *net.UDPAddr, username[]byte, n int) (*ClientSession, error){
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	key := s.sessionHashKey(addr, username, n)
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
	clientSession.addr = addr
	clientSession.hashKey = s.sessionHashKey(addr, ar.Username[:], len(ar.Username))
	clientSession.Init(s.server, s.server.PendingWriteNum)
	user.auth_session = clientSession
	clientSession.user = user

	s.mutexConns.Lock()
	s.sessions[cid] = clientSession
	s.sessionHash[clientSession.hashKey] = clientSession
	s.SessionCount++
	s.mutexConns.Unlock()

	s.wgConns.Add(1)
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

	s.DeleteSession(session)
	return true
}

func (s *UDPSessionManager) clientSessionLookUpByUsername(addr *net.UDPAddr, username []byte, n int) (*ClientSession, error) {
	session, err := s.sessionHashLookUp(addr, username, n)
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

func (s *UDPSessionManager) clientUserLookUp(username []byte, n int) (*ClientUser, error) {
	s.mutexConns.Lock()
	defer s.mutexConns.Unlock()

	key,_ := util.ByteToString(username, n)

	if user, ok := s.users[key];ok {
		return user, nil
	}
	User := &ClientUser{}
	util.ByteCopy(User.username[:], username, n)

	s.users[key] = User

	return User, nil
}

func (s *UDPSessionManager) clientTryUpdateIp(session *ClientSession, addr *net.UDPAddr){
	if session.addr.String() != addr.String() {
		s.mutexConns.Lock()
		delete(s.sessionHash, session.hashKey)
		session.hashKey = s.sessionHashKey(addr, session.user.username[:], len(session.user.username))
		s.sessionHash[session.hashKey] = session
		session.addr = addr
		s.mutexConns.Unlock()
	}
}

func (s *UDPSessionManager) ClientSessionGet(buf []byte, n int, addr *net.UDPAddr) (*ClientSession, error) {
	ucph := &receiver.Ucph{}
	ar := &receiver.ClientAuthRequest{}

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

		session, err := s.clientSessionLookUpByUsername(addr, ar.Username[:], len(ar.Username))
		if session != nil && err == nil {
			log.Debug("client_session_lookup: ok, ip,port:%s\n", addr.String())
			util.ByteCopy(session.rand1[:], ar.Rand1[:], len(ar.Rand1))
			return session, nil
		}

		user, err := s.clientUserLookUp(ar.Username[:], len(ar.Username))
		if err != nil {
			return nil, fmt.Errorf("User %s not found\n", ar.Username)
		}

		if(user.estab_session != nil && user.estab_session.addr.String() == addr.String()) {
			log.Debug("esta_session is not NULL, s->cid:%u\n", user.estab_session.cid);
			return user.estab_session, nil
		}

		if(user.auth_session != nil) {
			if user.auth_session.addr.String() == addr.String() {
				log.Debug("auth_session ip:%s,sock:%d, s->cid:%d\n", addr.String(), user.auth_session.cid);
				util.ByteCopy(user.auth_session.rand1[:], ar.Rand1[:], len(ar.Rand1))
				return user.auth_session, nil
			}else {
				log.Debug("usersession:0x%08X have new device connect(%s), delete old one(%s)\n",
					user.auth_session.cid, addr.String(), user.auth_session.addr.String());
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
func (s *UDPSessionManager) RouteToSession(buf []byte, n int, addr *net.UDPAddr) {
	session, err := s.ClientSessionGet(buf, n, addr)
	if err != nil {
		return
	}
	s.routeMsgToSession(session, buf, n)
}

func (s *UDPSessionManager) routeMsgToSession(session *ClientSession, buf []byte, n int) {
	b := make([]byte, n)
	util.ByteCopy(b, buf, n)
	session.TransMsg(b)
}

func (s *UDPSessionManager) Stop() {
	s.mutexConns.Lock()
	for _, v := range s.sessions {
		v.Close()
	}

	s.SessionCount = 0
	s.cid = 1
	s.sessions = make(SessionMap)
	s.sessionHash = make(SessionHash)
	s.users = make(SessionUser)
	s.mutexConns.Unlock()

	s.wgConns.Wait()
}

func (s *ClientSession) Destory(sendReset bool) {
	if sendReset {
		//send reset
	}

	s.UDPConn.Close()
}
