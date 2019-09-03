// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"
#include <vector>
#include <list>
#include <unordered_map>

class OverlappedIOService : public IOServiceBase
{
public:
    OverlappedIOService();
    ~OverlappedIOService();

    int AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo, OverlapCallback cb);
    int AsyncAccept(OverlapContext* ctx, OverlapCallback cb);
    int AsyncRead(OverlapContext* ctx, OverlapCallback cb);
    int AsyncWrite(OverlapContext* ctx, OverlapCallback cb);

    OverlapContext* AllocOverlapCtx();
    void FreeOverlapCtx(OverlapContext* ctx);

    int Run(int timeout);

private:
    void CleanUp();
    void DispatchEvent(OverlapContext* ctx);
    
private:
    std::list<OverlapContext*>      pool_;
    std::vector<WSAEVENT>           events_;
    std::unordered_map<WSAEVENT, OverlapContext*> fds_;
};