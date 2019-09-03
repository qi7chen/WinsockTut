// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <stdint.h>
#include "TimerSched.h"
#include "OverlapFd.h"


enum IOServiceType
{
    IOOverlapped = 1,
    IOAlertable = 2,
    IOCompletionPort = 3,
};

// Async I/O service for TCP sockets
class IOServiceBase : public TimerSched
{
public:
    IOServiceBase();
    virtual ~IOServiceBase();

    virtual int AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo, OverlapCallback cb) = 0;
    virtual int AsyncAccept(OverlapContext* ctx, OverlapCallback cb) = 0;
    virtual int AsyncRead(OverlapContext* ctx, OverlapCallback cb) = 0;
    virtual int AsyncWrite(OverlapContext* ctx, OverlapCallback cb) = 0;

    virtual int Run(int timeout) = 0;

    virtual OverlapContext* AllocOverlapCtx() = 0;
    virtual void FreeOverlapCtx(OverlapContext* ctx) = 0;
};

IOServiceBase* CreateIOService(IOServiceType type);
