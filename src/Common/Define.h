// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <functional>

enum
{
    EV_NONE = 0,
    EV_READABLE = 1,
    EV_WRITABLE = 2,
};

enum 
{
    TypeSelect = 1,
};

typedef std::function<void(SOCKET,int)> EventProc;

class EventLoop;

struct IMultiplexer
{
    virtual void AddFd(SOCKET fd, int mask) = 0;
    virtual void DelFd(SOCKET fd, int mask) = 0;
    virtual int Poll(EventLoop* loop, int timeout) = 0;
};
