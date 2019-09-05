// Copyright (C) 2012-present All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

package main

import (
	"bytes"
	"log"
	"net"
	"sync"
	"time"
)

type ChatServer struct {
	done     chan struct{}  // done signal
	wg       sync.WaitGroup //
	ln       net.Listener   //
	guard    sync.RWMutex
	sessions map[int]net.Conn
	counter  int
}

func NewChatServer() *ChatServer {
	return &ChatServer{
		done:     make(chan struct{}),
		sessions: make(map[int]net.Conn, 1000),
		counter:  2001,
	}
}

func (s *ChatServer) Start(addr string) {
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		log.Fatalf("Listen: %v", err)
	}
	s.ln = ln
	log.Printf("listen at %s\n", addr)
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Printf("Accept: %v\n", err)
			return
		}
		log.Printf("accepted %v\n", conn.RemoteAddr())

		s.guard.Lock()
		var counter = s.counter
		s.counter++
		s.sessions[counter] = conn
		s.guard.Unlock()
		s.wg.Add(1)
		go s.handleNewConn(counter, conn)
	}
}

func (s *ChatServer) Close() {
	close(s.done)
	s.ln.Close()
	s.guard.RLock()
	for _, conn := range s.sessions {
		conn.Close()
	}
	s.guard.RUnlock()
	s.wg.Wait()
}

func (s *ChatServer) CloseSession(id int) {
	s.guard.Lock()
	if conn := s.sessions[id]; conn != nil {
		delete(s.sessions, id)
		conn.Close()
	}
	s.guard.Unlock()
}

func (s *ChatServer) Broadcast(command string, args ...interface{}) {
	var buf bytes.Buffer
	EncodeMessage(&buf, command, args)
	s.guard.RLock()
	for id, conn := range s.sessions {
		if _, err := conn.Write(buf.Bytes()); err != nil {
			log.Printf("Write: %d, %v\n", id, err)
		}
	}
	s.guard.RUnlock()
}

func (s *ChatServer) handleNewConn(id int, conn net.Conn) {
	defer s.wg.Done()
	defer s.CloseSession(id)

	conn.SetReadDeadline(time.Now().Add(time.Minute * 2))
	messages, err := ReadMessage(conn)
	if err != nil {
		log.Printf("ReadMessage: %v\n", err)
		return
	}
	if len(messages) < 2 {
		log.Printf("invalid message format: %v\n", messages)
		return
	}

	var nickname = messages[1]
	WriteMessage(conn, "REG_ACK")
	log.Printf("%s entered\n", nickname)
	s.Broadcast("ENTER", nickname)

	for {
		conn.SetReadDeadline(time.Now().Add(time.Minute * 2))
		messages, err := ReadMessage(conn)
		if err != nil {
			log.Printf("ReadMessage: %v\n", err)
			return
		}
		var command = messages[0]
		var content string
		if len(messages) > 1 {
			content = messages[1]
		}
		s.Broadcast(command, content)
	}
}
