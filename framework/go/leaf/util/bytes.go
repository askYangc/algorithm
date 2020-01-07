package util

import (
	"bytes"
	"fmt"
)

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

func ByteCopy(dst []byte, src []byte, n int) error{
	if len(dst) < n || len(src) < n {
		return fmt.Errorf("dst len(%d) or src len(%d) < %d\n", len(dst), len(src), n)
	}
	for i := 0; i < n; i++ {
		dst[i] = src[i]
	}

	return nil
}

func ByteClone(src []byte, n int) []byte{
	if len(src) < n {
		return nil
	}

	dst := make([]byte, n)

	for i := 0; i < n; i++ {
		dst[i] = src[i]
	}

	return dst
}

func StringToByte(dst []byte, src string, n int) error{
	if len(dst) < len(src) || len(src) < n {
		return fmt.Errorf("dst len(%d) or src len(%d) < %d, n %d\n", len(dst), len(src), n)
	}

	for i := 0; i < n; i++ {
		dst[i] = src[i]
	}

	return nil
}

func ByteToString(src []byte, n int) (string, error){
	if len(src) < n {
		return "", fmt.Errorf("src len(%d) < %d\n", len(src), n)
	}

	str := string(src[:n])

	return str, nil
}