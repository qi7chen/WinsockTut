package main

import (
	"log"
	"net"
	"sync"
)

type ChatClient struct {
	Transport
	wg       *sync.WaitGroup
	inbound  chan *ChatMessage
	outbound chan *ChatMessage
}

func NewChatClient(wg *sync.WaitGroup, conn net.Conn) *ChatClient {
	var c = &ChatClient{
		wg:       wg,
		inbound:  make(chan *ChatMessage, 1024),
		outbound: make(chan *ChatMessage, 1024),
	}
	c.Transport.Init(conn, c.inbound, c.outbound)
	return c
}

func (c *ChatClient) Go() {
	c.Transport.Go()
	go c.run()
}

func (c *ChatClient) run() {
	defer c.wg.Done()
	for {
		select {
		case msg := <-c.inbound:
			c.handleCommand(msg)
		}
	}
}

func (c *ChatClient) Send(command, name, content string) {
	var msg = NewChatMessage()
	msg.command = command
	msg.name = name
	msg.content = content
	select {
	case c.outbound <- msg:
	default:
		log.Printf("out buffer overflow")
	}
}

func (c *ChatClient) handleCommand(msg *ChatMessage) {
	switch msg.command {

	}
}
