#! /usr/bin/env python
# -*- coding: utf-8 -*-

import socket
import select
import pdb

def fd_handler(fd):
    msg = fd.recv(1024)
    if len(msg) == 0: 
        fd.close()
    else:
        print(msg)
        fd.send(msg)

def create_connections(host, port, count):
    mydic = {}
    for x in range(count):
        s = socket.create_connection((host, port))
        s.setblocking(0)
        mydic[s] = fd_handler
    return mydic

def dispatch(mydict):
    readlist = mydict.keys()
    while True:
        infds, outfds, errfds = select.select(readlist, [], [], 0)
        if len(infds) == 0: break
        for fd in infds:
            handler = mydict.get(fd)
            if handler: 
                handler(fd)
            else: 
                fd.close()
    

def test_case(host, port, count):
    msg = 'a quick fox jumps over the lazy dog\n'
    mydict = create_connections(host, port, count)
    for s in mydict.keys():
        s.send(msg)
    dispatch(mydict)

if __name__ == '__main__':
    host = '192.168.0.9'
    port = 3245
    count = 1
    test_case(host, port, count)
    
