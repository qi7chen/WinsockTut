// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <functional>

enum
{
    EV_NONE = 0,
    EV_READABLE = 0x1,
    EV_WRITABLE = 0x2,
    EV_EXCEPT = 0x10,
};

enum IOMode
{
    IOSelect = 1,
    IOAsyncSelect = 2,
    IOEventSelect = 3,
};

class EventLoop;

typedef std::function<void(EventLoop*, SOCKET, int)> EventProc;

struct IOPoller
{
    virtual void AddFd(SOCKET fd, int mask) = 0;
    virtual void DelFd(SOCKET fd, int mask) = 0;
    virtual int Poll(EventLoop* loop, int timeout) = 0;
};
