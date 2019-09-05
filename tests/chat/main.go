// Copyright (C) 2012-present All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

package main

import (
	"flag"
	"log"
)

func main() {
	var mode, addr string
	flag.StringVar(&mode, "m", "server", "run server or client")
	flag.StringVar(&addr, "h", "127.0.0.1:8081", "host address")
	flag.Parse()

	if mode == "client" {
		var client = NewChatClient()
		client.Start(addr)
	} else if mode == "server" {
		var server = NewChatServer()
		server.Start(addr)
	} else {
		log.Panicf("invalid mode: %s", mode)
	}
}
