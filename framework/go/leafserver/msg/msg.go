package msg

import (
	"leaf/network/example/json"
)

var Processor = json.NewProcessor()

func init() {
	Processor.Register(&Hello{})
}

type Hello struct {
	Name string
}