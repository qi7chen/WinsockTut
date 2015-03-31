#! /usr/bin/env python
# -*- coding: utf-8 -*-

#
# Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
# Distributed under the terms and conditions of the Apache License. 
# See accompanying files LICENSE.
#

import socket
import asyncore
import random
import pdb


class tcp_client(asyncore.dispatcher):
    def __init__(self, host, port, msg):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect((host, port))
        self.buffer = msg
        self.send_bytes = 0
        self.recv_bytes = 0
        

    def handle_connect(self):
        print(repr(self), 'connected')

    def handle_close(self):
        print(repr(self), 'bytes sent:', self.send_bytes, 'bytes read:', self.recv_bytes)
        self.close()

    def handle_read(self):
        data = self.recv(8192)
        self.recv_bytes += len(data)

    def writable(self):
        return (len(self.buffer) > 0)

    def handle_write(self):
        sent = self.send(self.buffer)
        self.send_bytes += sent
        

def create_clients(host, port, count, msg):
    clients = []    
    for i in range(count):
        c = tcp_client(host, port, msg)
        clients.append(c)
    return clients          


def run_test():
    host = '127.0.0.1'
    port = 32450
    maxcount = 512  # max 512
    msg = 'GET /index.html HTTP/1.0\r\n\r\n'
    count = random.randint(1, maxcount)
    print(count, 'testing client')
    clients = create_clients(host, port, count, msg)
    asyncore.loop()

if __name__ == '__main__':
    run_test()



