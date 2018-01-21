package main

import (
	"log"
	"net"
	"os"
	"strconv"
	"sync"
	"time"
)

var maxCount = 6

func main() {
	var addr = "localhost:8081"
	var count = 63
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
    if len(os.Args) > 3 {
		i, err := strconv.Atoi(os.Args[2])
		if err != nil {
			log.Fatalf("strconv: %v", err)
		}
        maxCount = i
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
	for i := 0; i < maxCount; i++ {
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
