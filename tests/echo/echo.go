// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

package main

import (
	"flag"
	"log"
	"net"
	"sync"
	"time"
)

func main() {
	var mode, addr string
	var connCount, echoCount int
	flag.StringVar(&mode, "m", "server", "run server or client")
	flag.StringVar(&addr, "h", "127.0.0.1:8081", "host address")
	flag.IntVar(&connCount, "c", 32, "how many connections to make")
	flag.IntVar(&echoCount, "e", 5, "server response echo back count")
	flag.Parse()

	switch mode {
	case "client":
		startClient(addr, connCount, echoCount)
	case "server":
		startServer(addr, echoCount)
	default:
		log.Panicf("invalid mode: %s", mode)
	}
}

// 开始跑echo client
func startClient(addr string, connCount, echoCount int) {
	log.Printf("starting %d clients to connect %s\n", connCount, addr)
	var wg sync.WaitGroup
	for i := 0; i < connCount; i++ {
		conn, err := net.Dial("tcp", addr)
		if err != nil {
			log.Fatalf("Dial: %v", err)
		}
		wg.Add(1)
		go handleClientConn(conn, echoCount, &wg)
		time.Sleep(10 * time.Millisecond)
	}
	wg.Wait()
}

//连续发送/接收`echoCount`条消息就close
func handleClientConn(conn net.Conn, echoCount int, wg *sync.WaitGroup) {
	defer wg.Done()
	defer conn.Close()

	var msg = []byte("a quick brown fox jumps over the lazy dog")
	var buf = make([]byte, 1024)
	for i := 0; i < echoCount; i++ {
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

// 开始跑echo server
func startServer(addr string, echoCount int) {
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
		go handleServerConn(conn, echoCount)
	}
}

//连续接收/发送`echoCount`条消息就close
func handleServerConn(conn net.Conn, echoCount int) {
	defer conn.Close()
	defer log.Printf("connection %v closed\n", conn.RemoteAddr())
	var buf = make([]byte, 1024)
	for i := 0; i < echoCount; i++ {
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
