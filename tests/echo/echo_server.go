package main

import (
	"flag"
	"log"
	"net"
	"time"
)

func main() {
	var addr string
	flag.StringVar(&addr, "", ":8081", "server address to bind")
	flag.Parse()

	ln, err := net.Listen("tcp", addr)
	if err != nil {
		log.Fatalf("Listen: %v", err)
	}
	log.Printf("listen at %s\n", addr)
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Fatalf("Accept: %v", err)
		}
		log.Printf("accepted %v\n", conn.RemoteAddr())
		go handleConn(conn)
	}
}

func handleConn(conn net.Conn) {
	defer conn.Close()
	defer log.Printf("connection %v closed\n", conn.RemoteAddr())
	var buf = make([]byte, 1024)
	for {
		conn.SetDeadline(time.Now().Add(300 * time.Second))
		nbytes, err := conn.Read(buf)
		if err != nil {
			log.Printf("Read: %v\n", err)
			break
		}
		if nbytes == 0 {
			log.Printf("Recv 0 bytes, EOF\n")
			break
		}
		log.Printf("Recv %d bytes\n", nbytes)
		nbytes, err = conn.Write(buf[:nbytes])
		if err != nil {
			log.Printf("Write: %v\n", err)
			break
		}
		log.Printf("Send %d bytes\n", nbytes)
	}
}
