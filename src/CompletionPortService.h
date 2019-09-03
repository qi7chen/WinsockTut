// Copyright (C) 2012-present prototyped.cn All rights reserved.
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

    //int AsyncConnect(SOCKET fd, const addrinfo* pinfo, ConnectCallback cb);
    //int AsyncAccept(SOCKET acceptor, AcceptCallback cb);
    //int AsyncRead(void* buf, int size, ReadCallback cb);
    //int AsyncWrite(const void* buf, int size, WriteCallback cb);
    //int CancelFd(SOCKET fd);
    //int Run(int timeout);

private:
    HANDLE  completion_port_;
};