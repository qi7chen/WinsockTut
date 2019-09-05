// Copyright (C) 2012-present All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"
)

type ChatClient struct {
	conn     net.Conn
	nickname string
}

func NewChatClient() *ChatClient {
	return &ChatClient{}
}

func (c *ChatClient) Close() {
	if c.conn != nil {
		c.conn.Close()
		c.conn = nil
	}
}

func (c *ChatClient) Start(addr string) {
	log.Printf("starting to connect %s\n", addr)
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		log.Printf("Dial: %v\n", err)
		return
	}
	c.conn = conn
	defer c.Close()

	myname, err := c.readConsole("Enter your name: ")
	if err != nil {
		log.Printf("%v\n", err)
		return
	}
	c.nickname = myname
	WriteMessage(c.conn, "REG_REQ", myname)
	_, err = ReadMessage(c.conn)
	if err != nil {
		log.Printf("%v\n", err)
		return
	}

	go c.writeConsoleInput()
	c.readChatMessage()
}

func (c *ChatClient) readChatMessage() {
	for {
		messages, err := ReadMessage(c.conn)
		if err != nil {
			log.Printf("Read: %v\n", err)
			return
		}
		if len(messages) < 1 {
			log.Printf("invalid message format: %v\n", messages)
			return
		}
		switch messages[0] {
		case "ENTER":
			fmt.Printf("%v entered \n", messages[1:])
		case "MSG_REQ":
			fmt.Printf("%v\n", messages[1:])
		}
	}
}

func (c *ChatClient) readConsole(tips string) (string, error) {
	reader := bufio.NewReader(os.Stdin)
	fmt.Print(tips)
	text, err := reader.ReadString('\n')
	if err != nil {
		return "", err
	}
	return text, nil
}

func (c *ChatClient) writeConsoleInput() {
	for {
		content, err := c.readConsole("Enter chat message: ")
		if err != nil {
			log.Printf("exit chat, %v\n", err)
			return
		}
		content = strings.TrimSpace(content)
		if len(content) > 0 {
			if err = WriteMessage(c.conn, "MSG_REQ", c.nickname, content); err != nil {
				log.Printf("exit chat, %v\n", err)
				return
			}
		}
		time.Sleep(time.Second)
	}
}

func (c *ChatClient) signaler() {
	var sigChan = make(chan os.Signal)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGABRT)
	for {
		select {
		case sig := <-sigChan:
			switch sig {
			case syscall.SIGINT, syscall.SIGABRT:
				log.Printf("signal %s received\n", sig)
				c.Close()
			}
		}
	}
}
