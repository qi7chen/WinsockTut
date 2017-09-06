package main

import (
	"flag"
	"log"
	"net"
	"sync"
)

func main() {
	var hostAddress string
	var totalCount int
	flag.StringVar(&hostAddress, "H", "localhost", "host server address, eg: hello.com:12345")
	flag.IntVar(&totalCount, "C", 1, "total client count")

	var wg sync.WaitGroup
	for i := 0; i < totalCount; i++ {
		conn, err := net.Dial("tcp", hostAddress)
		if err != nil {
			log.Printf("Dial: %v", err)
		} else {
			wg.Add(1)
			var client = NewChatClient(&wg, conn)
			client.Go()
		}
	}
	wg.Wait()
}
