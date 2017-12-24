// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <unordered_map>
#include "Common/Define.h"
#include "EventLoop.h"


class EchoServer
{
public:
    explicit EchoServer(IOMode mode);
    ~EchoServer();

    void Start(const char* host, const char* port);
    void Run();

private:
    void OnAccept(EventLoop* loop, SOCKET fd, int mask);

private:
    EventLoop*  loop_;
    SOCKET      acceptor_;
    //std::unordered_map<SOCKET>
};
