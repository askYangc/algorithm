package receiver

import (
	"bytes"
	"encoding/binary"
	"leaf/util"
)


type Ucph struct {
	Ver uint8
	Hlen uint8
	Request uint8
	Encrypt uint8

	Flag uint8
	Request_id uint8
	Fragments uint8

	Client_sid uint32
	Command uint16
	Param_len uint16
}

type ClientAuthRequest struct {
	Action uint8
	Version uint8
	Reserved [2]byte
	Rand1 [4]byte
	Username [16]byte
	Uuid [16]byte
}

func NewUcph(cmd uint16, param_len uint16, is_request uint8, is_enc uint8, flag uint8) (*Ucph){
	hdr := &Ucph{}

	hdr.Ver = 1
	hdr.Hlen = uint8(hdr.GetLen()/4)
	hdr.Command = cmd
	hdr.Param_len = param_len
	hdr.Request = is_request
	hdr.Encrypt = is_enc
	hdr.Flag = flag

	return hdr
}

func (ar *ClientAuthRequest) GetLen() (uint32) {
	return 40
}

func (ar *ClientAuthRequest) Pack() ([]byte, error) {
	var buffer bytes.Buffer

	buffer.WriteByte(ar.Action)
	buffer.WriteByte(ar.Version)
	buffer.WriteByte(ar.Reserved[0])
	buffer.WriteByte(ar.Reserved[1])

	si := ar.Rand1[:]
	buffer.Write(si)
	si = ar.Username[:]
	buffer.Write(si)
	si = ar.Uuid[:]
	buffer.Write(si)

	return buffer.Bytes(), nil
}

func (ar *ClientAuthRequest) Unpack(b []byte) (uint32, error) {
	ar.Action = b[0]
	ar.Version = b[1]
	ar.Reserved[0] = 0
	ar.Reserved[1] = 0

	offset := 4
	util.ByteCopy(ar.Rand1[:], b[offset:], len(ar.Rand1))

	offset += len(ar.Rand1)
	util.ByteCopy(ar.Username[:], b[offset:], len(ar.Username))
	ar.Username[15] = 0

	offset += len(ar.Username)
	util.ByteCopy(ar.Uuid[:], b[offset:], len(ar.Uuid))

	return ar.GetLen(), nil
}

func (hdr *Ucph) getFirstFromByte(b byte) {
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

func (hdr *Ucph) BuildFirst() (b byte) {
	if IsLittleEndian() == true {
		//b = byte(hdr.Ver | (hdr.Hlen << 3) | (hdr.Request << 6) | (hdr.Encrypt << 7))
		b = byte((hdr.Ver << 5) | (hdr.Hlen << 2) | (hdr.Request << 1) | (hdr.Encrypt))
	}else {
		b = byte(hdr.Ver | (hdr.Hlen << 3) | (hdr.Request << 6) | (hdr.Encrypt << 7))
	}
	return
}


func (hdr *Ucph) GetLen() (uint32) {
	return 12
}

func (hdr *Ucph) Pack() ([]byte, error) {
	var buffer bytes.Buffer
	buffer.WriteByte(hdr.BuildFirst())
	buffer.WriteByte(hdr.Flag)
	buffer.WriteByte(hdr.Request_id)
	buffer.WriteByte(hdr.Fragments)
	binary.Write(&buffer, binary.BigEndian, hdr.Client_sid)
	binary.Write(&buffer, binary.BigEndian, hdr.Command)
	binary.Write(&buffer, binary.BigEndian, hdr.Param_len)

	return buffer.Bytes(), nil
}

func (hdr *Ucph) Unpack(data []byte) (uint32, error) {
	hdr.getFirstFromByte(data[0])
	hdr.Flag = data[1]
	hdr.Request_id = data[2]
	hdr.Fragments = data[3]
	offset := 4
	hdr.Client_sid = binary.BigEndian.Uint32(data[offset:])
	offset += 4
	hdr.Command = binary.BigEndian.Uint16(data[offset:])
	offset += 2
	hdr.Param_len = binary.BigEndian.Uint16(data[offset:])

	return hdr.GetLen(), nil
}
