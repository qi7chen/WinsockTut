#! /usr/bin/env python
# -*- coding: utf-8 -*-

import socket
import select
import time
import pdb

# select loop
def event_loop(read_list):
    while read_list:
        rs, ws, es = select.select(read_list, [], [], 3)
        if not (rs or ws or es):
            print('timed out')
            continue
        for s in rs:
            data = s.recv(1024)
            #print(s.fileno(), data)
            s.send(data)
            time.sleep(0.5)

# create clients 
def create_client(endpoint, count):
    msg = 'the quick fox jumps over a lazy dog' * 32
    clients = []    
    for i in range(count):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(endpoint)
        print(s.fileno(), 'connected', endpoint)
        s.setblocking(False)
        s.send(msg)        
        clients.append(s)
    return clients


def run_test():
    host = '127.0.0.1'
    port = 3245
    count = 100
    clients = create_client((host, port), count)
    event_loop(clients)


if __name__ == '__main__':
    run_test()

    

