// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "Common/Define.h"
#include "Reactor/EventLoop.h"

class EchoClient
{
public:
    explicit EchoClient(IOMode mode);
    ~EchoClient();

    void Start(const char* host, const char* port);

    void Run();

private:
    void Cleanup();
    SOCKET Connect(const char* host, const char* port);
    void HandleEvent(SOCKET fd, int mask, int err);
    void OnReadable(SOCKET fd);
    void OnWritable(SOCKET fd);

private:
    SOCKET      fd_;
    EventLoop*  loop_;
};
