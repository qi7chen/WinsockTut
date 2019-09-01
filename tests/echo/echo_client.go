package main

import (
	"flag"
	"log"
	"net"
	"sync"
	"time"
)

var maxPktCount int

func main() {
	var addr string
	var connCount int
	flag.StringVar(&addr, "h", "localhost:8081", "server address to bind")
	flag.IntVar(&connCount, "c", 63, "connection count to make")
	flag.IntVar(&maxPktCount, "p", 10, "packet count to send")
	flag.Parse()

	var wg sync.WaitGroup
	for i := 0; i < connCount; i++ {
		conn, err := net.Dial("tcp", addr)
		if err != nil {
			log.Fatalf("Dial: %v", err)
		}
		wg.Add(1)
		go handleConn(conn, &wg)
		time.Sleep(10 * time.Millisecond)
	}
	wg.Wait()
}

func handleConn(conn net.Conn, wg *sync.WaitGroup) {
	defer wg.Done()
	defer conn.Close()

	var msg = []byte("a quick brown fox jumps over the lazy dog")
	var buf = make([]byte, 1024)
	for i := 0; i < maxPktCount; i++ {
		nbytes, err := conn.Write(msg)
		if err != nil {
			log.Printf("%v Read: %v\n", conn.LocalAddr(), err)
			break
		}
		if nbytes == 0 {
			log.Printf("%v Recv 0 bytes, EOF\n", conn.LocalAddr())
			break
		}
		log.Printf("%v Write %d bytes\n", conn.LocalAddr(), nbytes)
		nbytes, err = conn.Read(buf)
		if err != nil {
			log.Printf("Write: %v\n", err)
			break
		}
		log.Printf("%v Read %d bytes\n", conn.LocalAddr(), nbytes)
		time.Sleep(time.Second)
	}
}
