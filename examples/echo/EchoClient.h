// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "Common/Define.h"
#include "EventLoop.h"

class EchoClient
{
public:
    explicit EchoClient(IOMode mode);
    ~EchoClient();

    void Start(const char* host, const char* port);

    void Run();

private:
    bool Connect(const char* host, const char* port);
    void OnConnect(EventLoop* loop, SOCKET fd, int mask);
    void OnReadable(EventLoop* loop, SOCKET fd, int mask);
    void OnWritable(EventLoop* loop, SOCKET fd, int mask);

private:
    SOCKET      fd_;
    EventLoop*  loop_;
};
