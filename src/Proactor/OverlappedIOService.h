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

    int AsyncConnect(const std::string& addr, const std::string& port, ConnectCallback cb);
    int AsyncListen(SOCKET fd, const std::string& addr, const std::string& port, AcceptCallback cb);
    int AsyncRead(SOCKET fd, void* buf, int size, ReadCallback cb);
    int AsyncWrite(SOCKET fd, void* buf, int size, WriteCallback cb);

    int Poll(int timeout);

private:
    struct PerHandleData;

    PerHandleData* AllocHandleData(SOCKET fd);
    void FreeHandleData(PerHandleData* data);
    PerHandleData* GetHandleData(WSAEVENT hEvent);
    void CleanUp();

private:
    WSAEVENT events_[WSA_MAXIMUM_WAIT_EVENTS];
    std::unordered_map<HANDLE, SOCKET>  event_socks_;
    std::unordered_map<SOCKET, PerHandleData*>  sock_handles_;
};