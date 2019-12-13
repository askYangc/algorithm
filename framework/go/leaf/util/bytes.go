package util

import "bytes"

func ByteJoin(args ...[]byte) []byte{
	if len(args) == 0 {
		return make([]byte, 0)
	}

	var buffer bytes.Buffer
	for i := 0; i < len(args); i++ {
		buffer.Write(args[i])
	}

	return buffer.Bytes()
}

func StringJoin(args ...string) string{
	if len(args) == 0 {
		return ""
	}

	var buffer bytes.Buffer
	for i := 0; i < len(args); i++ {
		buffer.WriteString(args[i])
	}

	return buffer.String()
}