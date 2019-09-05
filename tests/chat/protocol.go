// Copyright (C) 2012-present All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"reflect"
	"strings"
	"time"
	"unsafe"
)

const (
	MAX_MESSAGE_LEN = 2048
	DELIM           = "\r\n"
)

func FastBytesToString(b []byte) string {
	bh := (*reflect.SliceHeader)(unsafe.Pointer(&b))
	sh := reflect.StringHeader{Data: bh.Data, Len: bh.Len}
	return *(*string)(unsafe.Pointer(&sh))
}

func EncodeMessage(buf *bytes.Buffer, command string, args ...interface{}) {
	buf.WriteString(command)
	if len(args) > 0 {
		buf.WriteString(DELIM)
		for i := 0; i < len(args); i++ {
			fmt.Fprintf(buf, "%v", args[i])
			if i+1 < len(args) {
				buf.WriteString(" ")
			}
		}
	}
}

func WriteMessage(conn net.Conn, command string, args ...interface{}) error {
	var buf bytes.Buffer
	EncodeMessage(&buf, command, args...)
	return WriteContent(conn, buf.Bytes())
}

func WriteContent(conn net.Conn, content []byte) error {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[0:], uint32(len(content)))
	if _, err := conn.Write(buf[0:]); err != nil {
		return err
	}
	_, err := conn.Write(content)
	return err
}

func ReadMessage(conn net.Conn) ([]string, error) {
	var lenbuf [4]byte
	conn.SetReadDeadline(time.Now().Add(time.Minute * 5))
	if _, err := io.ReadAtLeast(conn, lenbuf[0:], len(lenbuf)); err != nil {
		return nil, err
	}
	var length = binary.LittleEndian.Uint32(lenbuf[0:])
	if length > MAX_MESSAGE_LEN {
		return nil, fmt.Errorf("message content over size %d", length)
	}
	if length == 0 {
		return nil, fmt.Errorf("invalid message format")
	}
	var buf = make([]byte, length)
	conn.SetReadDeadline(time.Now().Add(time.Minute))
	if _, err := io.ReadAtLeast(conn, buf, int(length)); err != nil {
		return nil, err
	}
	var resp = strings.TrimSpace(FastBytesToString(buf))
	return strings.Split(resp, DELIM), nil
}
