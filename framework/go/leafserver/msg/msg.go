package msg

import (
	"leaf/network/processor/json"
)

var Processor = json.NewProcessor()

func init() {
	Processor.Register(&Hello{})
}

type Hello struct {
	Name string
}