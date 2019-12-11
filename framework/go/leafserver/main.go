package main

import (
	"leaf"
	lconf "leaf/conf"
	"leafserver/conf"
	"leafserver/gate"
	"leafserver/login"
)

func main() {
	lconf.LogLevel = conf.Server.LogLevel
	lconf.LogPath = conf.Server.LogPath
	lconf.LogFlag = conf.LogFlag
	lconf.ConsolePort = conf.Server.ConsolePort
	lconf.ProfilePath = conf.Server.ProfilePath

	leaf.Run(
		gate.Module,
		login.Module,
	)
}
