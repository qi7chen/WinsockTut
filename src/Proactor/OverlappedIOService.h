// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"
#include <unordered_map>

class OverlappedIOService : public IOServiceBase
{
public:
    OverlappedIOService();
    ~OverlappedIOService();

    int AsyncConnect(const char* addr, const char* port, ConnectCallback cb);
    int AsyncListen(OverlapFd* fd, const char* addr, const char* port, AcceptCallback cb);
    int AsyncRead(OverlapFd* fd, void* buf, int size, ReadCallback cb);
    int AsyncWrite(OverlapFd* fd, void* buf, int size, WriteCallback cb);

    int Poll(int timeout);

private:
    void DispatchEvent(OverlapFd* ev);
    OverlapFd* AllocOverlapFd(SOCKET fd);
    void FreeOverlapFd(OverlapFd* data);
    OverlapFd* GetOverlapFdByEvent(WSAEVENT hEvent);
    void CleanUp();

private:
    WSAEVENT events_[WSA_MAXIMUM_WAIT_EVENTS];
    std::unordered_map<HANDLE, SOCKET>  event_socks_;
    std::unordered_map<SOCKET, OverlapFd*>  sock_handles_;
};