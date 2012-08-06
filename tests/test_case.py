#! /usr/bin/env python
# -*- coding: utf-8 -*-


import socket
import timeit

def test_small_msg(host, port, loop_count):
    s = socket.create_connection((host, port))
    msg = 'a quick fox jumps over the lazy dog\n'
    for _ in range(loop_count):
        s.send(buffer(msg))
        data = s.recv(1024)
        if data != msg:
            print('test small message failed', repr(data))
            return False
    print('test small message passed')
    return True

def test_large_data(host, port, loop_count):
    s = socket.create_connection((host, port))
    # 36kb
    msg = 'a quick fox jumps over the lazy dog\n' * 1024
    for _ in range(loop_count):
        s.send(msg)
        data = s.recv(102400)
        if data != msg:
            print('test large message failed', repr(data))
            return False
    print('test large message passed')
    return True


    
if __name__ == '__main__':
    test_small_msg('127.0.0.1', 3245, 2)
    #test_case = "test_small_msg('192.168.0.59', 3245, 1)"
    #t1 = timeit.Timer(test_case, "from __main__ import test_small_msg")
    #print(t1.timeit(10))
    
