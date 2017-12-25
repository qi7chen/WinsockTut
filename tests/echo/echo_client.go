package main

import (
	"log"
	"net"
	"os"
	"strconv"
	"sync"
	"time"
)

func main() {
	var addr = "localhost:3001"
	var count = 60
	if len(os.Args) > 1 {
		addr = os.Args[1]
	}
	if len(os.Args) > 2 {
		i, err := strconv.Atoi(os.Args[2])
		if err != nil {
			log.Fatalf("strconv: %v", err)
		}
		count = i
	}
	var wg sync.WaitGroup
	for i := 0; i < count; i++ {
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
	for i := 0; i < 10; i++ {
		nbytes, err := conn.Write(msg)
		if err != nil {
			log.Printf("Read: %v\n", err)
			break
		}
		if nbytes == 0 {
			log.Printf("Recv 0 bytes, EOF\n")
			break
		}
		log.Printf("Write %d bytes\n", nbytes)
		nbytes, err = conn.Read(buf)
		if err != nil {
			log.Printf("Write: %v\n", err)
			break
		}
		log.Printf("Read %d bytes\n", nbytes)
		time.Sleep(time.Second)
	}
}
