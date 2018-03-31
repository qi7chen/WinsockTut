// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <stdint.h>
#include <functional>
#include <set>

typedef std::function<void(int,int)> ReadCallback;
typedef std::function<void(int,int)> WriteCallback;
typedef std::function<void(int)> ConnectCallback;
typedef std::function<void(int)> AcceptCallback;
typedef std::function<void()> TimerCallback;

class IOServiceBase
{
public:
    IOServiceBase();
    virtual ~IOServiceBase();

    virtual void AsyncConnect(SOCKET fd, void* buf, int size, ConnectCallback cb) = 0;
    virtual void AsyncAccept(SOCKET fd, void* buf, int size, AcceptCallback cb) = 0;
    virtual void AsyncRead(SOCKET fd, void* buf, int size, ReadCallback cb) = 0;
    virtual void AsyncWrite(SOCKET fd, void* buf, int size, WriteCallback cb) = 0;

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
