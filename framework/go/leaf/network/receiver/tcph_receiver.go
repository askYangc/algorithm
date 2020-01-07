package receiver

import (
	"bytes"
	"encoding/binary"
	"io"
	"leaf/network/tcp"
	"unsafe"
)

func IsLittleEndian() bool {
	n := 0x1234
	return *(*byte)(unsafe.Pointer(&n)) == 0x34
}

type TCPTcphReceiver struct {

}

type Tcph struct {
	Ver uint8
	Hlen uint8
	Request uint8
	Encrypt uint8

	Flag uint8
	Command uint16
	Param_len uint32
	rsv uint32
}
func NewTCPTcphReceiver() *TCPTcphReceiver {
	p := new(TCPTcphReceiver)
	return p
}

func (hdr *Tcph) getFirstFromByte(b byte) {
	var first uint8
	first = uint8(b)
	if IsLittleEndian() == true {
		hdr.Encrypt = first & 0x1
		hdr.Request = (first >> 1) & 0x1
		hdr.Hlen = (first >> 2) & 0x7
		hdr.Ver = (first >> 5) & 0x7
	}else {
		hdr.Ver = first & 0x7
		hdr.Hlen = (first >> 3) & 0x7
		hdr.Request = (first >> 6) & 0x1
		hdr.Encrypt = (first >> 7) & 0x1

	}
}

func (hdr *Tcph) BuildFirst() (b byte) {
	if IsLittleEndian() == true {
		//b = byte(hdr.Ver | (hdr.Hlen << 3) | (hdr.Request << 6) | (hdr.Encrypt << 7))
		b = byte((hdr.Ver << 5) | (hdr.Hlen << 2) | (hdr.Request << 1) | (hdr.Encrypt))
	}else {
		b = byte(hdr.Ver | (hdr.Hlen << 3) | (hdr.Request << 6) | (hdr.Encrypt << 7))
	}
	return
}


func (hdr *Tcph) GetLen() (uint32) {
	return 12
}

func (hdr *Tcph) Pack() ([]byte, error) {
	var buffer bytes.Buffer
	buffer.WriteByte(hdr.BuildFirst())
	buffer.WriteByte(hdr.Flag)
	binary.Write(&buffer, binary.BigEndian, hdr.Command)
	binary.Write(&buffer, binary.BigEndian, hdr.Param_len)
	binary.Write(&buffer, binary.BigEndian, hdr.rsv)

	return buffer.Bytes(), nil
}

func (hdr *Tcph) Unpack(data []byte) (uint32, error) {
	hdr.getFirstFromByte(data[0])
	hdr.Flag = data[1]
	offset := 2
	hdr.Command = binary.BigEndian.Uint16(data[offset:])
	offset += 2
	hdr.Param_len = binary.BigEndian.Uint32(data[offset:])
	offset += 4
	hdr.rsv = binary.BigEndian.Uint32(data[offset:])

	return hdr.GetLen(), nil
}

// goroutine safe
func (p *TCPTcphReceiver) Read(c interface{}) ([]byte, error) {
	var buffer bytes.Buffer
	tcph := &Tcph{}

	tcpConn := c.(tcp.TCPConn)

	bufMsgLen := make([]byte, tcph.GetLen())

	// read head
	if _, err := io.ReadFull(tcpConn.Conn, bufMsgLen); err != nil {
		return nil, err
	}

	tcph.Unpack(bufMsgLen)

	remain := make([]byte, tcph.Param_len)

	// read remain
	if _, err := io.ReadFull(tcpConn.Conn, remain); err != nil {
		return nil, err
	}

	buffer.Write(bufMsgLen)
	buffer.Write(remain)
	all := buffer.Bytes()
	return all, nil
}

// goroutine safe
func (p *TCPTcphReceiver) Write(c interface{}, args ...[]byte) error {
	tcpConn := c.(tcp.TCPConn)
	for i := 0; i < len(args); i++ {
		tcpConn.Conn.Write(args[i])
	}

	return nil
}