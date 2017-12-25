// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <unordered_map>
#include "Common/Define.h"
#include "Reactor/EventLoop.h"


class EchoServer
{
public:
    struct Connection
    {
        int cap;
        int size;
        char buf[1];
    };

public:
    explicit EchoServer(IOMode mode);
    ~EchoServer();

    void Start(const char* host, const char* port);
    void Run();

private:
    void Cleanup(SOCKET fd);
    void StartRead(SOCKET fd);
    void OnAccept(SOCKET fd, int mask, int err);
    void OnReadable(SOCKET fd, int mask, int err);
    void OnWritable(SOCKET fd, int mask, int err);

private:
    EventLoop*  loop_;
    SOCKET      acceptor_;
    std::unordered_map<SOCKET, Connection*> connections_;
};
