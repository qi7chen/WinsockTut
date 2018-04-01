// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <stdint.h>
#include <set>
#include "Operation.h"


enum IOServiceType
{
    Overlapped = 1,
    CompletionPort = 2,
};

// Asynchounous I/O service for TCP sockets
class IOServiceBase
{
public:
    IOServiceBase();
    virtual ~IOServiceBase();

    virtual int AsyncConnect(const std::string& addr, const std::string& port, ConnectCallback cb) = 0;
    virtual int AsyncListen(SOCKET fd, const std::string& addr, const std::string& port, AcceptCallback cb) = 0;
    virtual int AsyncRead(SOCKET fd, void* buf, int size, ReadCallback cb) = 0;
    virtual int AsyncWrite(SOCKET fd, void* buf, int size, WriteCallback cb) = 0;

    virtual int Poll(int timeout) = 0;

    int AddTimer(int millsec, TimerCallback cb);
    void CancelTimer(int id);
    void UpdateTimer();

protected:
    void clear();

    struct TimerEntry
    {
        int             id;
        int64_t         expire;
        TimerCallback   cb;

        bool operator < (const IOServiceBase::TimerEntry& b) const
        {
            return expire < b.expire;
        }
    };

    int counter_;
    std::multiset<TimerEntry>   tree_;
};

IOServiceBase* CreateIOService(IOServiceType type);
