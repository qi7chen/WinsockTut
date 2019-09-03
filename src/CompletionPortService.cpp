// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "CompletionPortService.h"

CompletionPortService::CompletionPortService()
{

}

CompletionPortService::~CompletionPortService()
{

}

int CompletionPortService::AsyncConnect(SOCKET fd, const addrinfo* pinfo, ConnectCallback cb)
{
    return 0;
}

int CompletionPortService::AsyncAccept(SOCKET acceptor, AcceptCallback cb)
{
    return 0;
}

int CompletionPortService::AsyncRead(void* buf, int size, ReadCallback cb)
{
    return 0;
}

int CompletionPortService::AsyncWrite(const void* buf, int size, WriteCallback cb)
{
    return 0;
}

int CompletionPortService::CancelFd(SOCKET fd)
{
    return 0;
}

int CompletionPortService::Run(int timeout)
{
    return 0;
}
