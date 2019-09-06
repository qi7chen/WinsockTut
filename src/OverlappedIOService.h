// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"
#include <vector>
#include <unordered_map>

class OverlappedIOService : public IOServiceBase
{
public:
    OverlappedIOService();
    ~OverlappedIOService();

    int AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo);
    int AsyncAccept(OverlapContext* ctx);
    int AsyncRead(OverlapContext* ctx);
    int AsyncWrite(OverlapContext* ctx);

    OverlapContext* AllocOverlapCtx(SOCKET fd, int flags);
    void FreeOverlapCtx(OverlapContext* ctx);

    int Run(int timeout);

private:
    void CleanUp();
    
private:
    std::vector<WSAEVENT>           events_;
    std::unordered_map<WSAEVENT, OverlapContext*> fds_;
};