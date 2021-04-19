// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

// I/O Completion Port
class CompletionPortService : public IOServiceBase
{
public:
    CompletionPortService();
    ~CompletionPortService();

    int AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo);
    int AsyncAccept(OverlapContext* ctx);
    int AsyncRead(OverlapContext* ctx);
    int AsyncWrite(OverlapContext* ctx);

    OverlapContext* AllocOverlapCtx(SOCKET fd, int flags);
    void FreeOverlapCtx(OverlapContext* ctx);

    int Run(int timeout);

private:
    HANDLE  completion_port_;
};