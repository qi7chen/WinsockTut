package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"strings"
)

const MAX_PAYLOAD_SIZE = 8192

var (
	CMD_ENTER_ROOM = "ENTR"
	CMD_LEAVE_ROOM = "LEAV"
	CMD_TALK_MSG   = "TALK"
)

type ChatMessage struct {
	command string
	name    string
	content string
}

func NewChatMessage() *ChatMessage {
	return &ChatMessage{}
}

func (m *ChatMessage) ReadFrom(rd io.Reader) error {
	var buf [2]byte
	if _, err := io.ReadFull(rd, buf[0:]); err != nil {
		return err
	}
	var size = binary.BigEndian.Uint16(buf[0:])
	if size > MAX_PAYLOAD_SIZE {
		return fmt.Errorf("payload size[%d] out of range", size)
	}
	var buffer = make([]byte, size)
	if _, err := io.ReadFull(rd, buffer); err != nil {
		return err
	}
	var text = string(buffer)
	var idx = strings.Index(text, "|")
	if idx <= 0 {
		return fmt.Errorf("invalid message %s", text)
	}
	m.command = text[:idx]
	text = text[idx+1:]
	if idx = strings.Index(text, "|"); idx > 0 {
		m.name = text[:idx]
		m.content = text[idx+1:]
	}
	return nil
}

func (m *ChatMessage) WriteTo(wr io.Writer) error {
	var size = len(m.command) + len(m.name) + len(m.content) + 2 // 2 dilimter
	if size > MAX_PAYLOAD_SIZE {
		return fmt.Errorf("payload size[%d] out of range", size)
	}
	var buf [2]byte
	binary.BigEndian.PutUint16(buf[0:], uint16(size))

	var buffer bytes.Buffer
	buffer.Write(buf[0:])
	buffer.WriteString(m.command)
	buffer.WriteByte('|')
	buffer.WriteString(m.name)
	buffer.WriteByte('|')
	buffer.WriteString(m.content)
	if _, err := wr.Write(buffer.Bytes()); err != nil {
		return err
	}
	return nil
}
