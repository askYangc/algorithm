package session

import "leaf/util"

type ClientUser struct {
	username [16]byte

	auth_session *ClientSession
	estab_session *ClientSession
}

func NewClientUser(username []byte) (*ClientUser,error) {
	user := &ClientUser{}
	util.ByteCopy(user.username[:], username, len(username))

	return user,nil
}
