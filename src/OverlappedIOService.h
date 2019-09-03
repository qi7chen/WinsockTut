// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"
#include <vector>

class OverlappedIOService : public IOServiceBase
{
public:
    OverlappedIOService();
    ~OverlappedIOService();

    int AsyncConnect(SOCKET fd, const addrinfo* pinfo, ConnectCallback cb);
    int AsyncAccept(SOCKET acceptor, AcceptCallback cb);
    int AsyncRead(void* buf, int size, ReadCallback cb);
    int AsyncWrite(const void* buf, int size, WriteCallback cb);
    int CancelFd(SOCKET fd);
    int Run(int timeout);

private:
    void CleanUp();
    void RemoveRetired();

    OverlapContext* FindContextEntryByEvent(WSAEVENT hEvent);
    void DispatchEvent(OverlapContext* ctx);
    OverlapContext* GetOrCreateOverlapContext(SOCKET fd, bool will_create);
    void FreeOverlapFd(OverlapContext* ctx);
    
private:
    std::vector<WSAEVENT>           events_;
    std::vector<OverlapContext*>    fds_;
    bool					        has_retired_;
};