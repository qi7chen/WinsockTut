package main

import (
	"log"
	"net"
	"sync"
)

type Transport struct {
	conn     net.Conn
	done     chan struct{}
	wg       sync.WaitGroup
	inbound  chan<- *ChatMessage
	outbound <-chan *ChatMessage
}

func (t *Transport) Init(conn net.Conn, inbound chan<- *ChatMessage, outbound <-chan *ChatMessage) {
	t.conn = conn
	t.done = make(chan struct{})
	t.inbound = inbound
	t.outbound = outbound
}

func (t *Transport) Go() {
	t.wg.Add(2)
	go t.reader()
	go t.writer()
}

func (t *Transport) Close() {
	t.conn.Close()
	close(t.done)
	t.wg.Wait()
}

func (t *Transport) reader() {
	defer t.wg.Done()
	for {
		var msg = NewChatMessage()
		if err := msg.ReadFrom(t.conn); err != nil {
			log.Printf("Read: %v", err)
			t.Close()
			return
		}
		select {
		case t.inbound <- msg:
		case <-t.done:
			return
		}
	}
}

func (t *Transport) writer() {
	defer t.wg.Done()
	for {
		select {
		case msg, ok := <-t.outbound:
			if !ok {
				return
			}
			if err := msg.WriteTo(t.conn); err != nil {
				log.Printf("Write: %v", err)
				t.Close()
				return
			}

		case <-t.done:
			return
		}
	}
}
