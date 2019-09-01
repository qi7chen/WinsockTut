// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <functional>

enum OperationType
{
    OpNone = 0,
    OpConnect = 1,
    OpAccept = 2,
    OpRead = 3, 
    OpWrite = 4,
    OpClose = 5,
};

struct OverlapFd
{
    WSAOVERLAPPED   overlap;
    WSABUF          buf;
    SOCKET          fd;
    OperationType   op;
    int             err;
    void*			ctx;

    OverlapFd()
    {
        memset(&overlap, 0, sizeof(overlap));
        buf.buf = NULL;
        buf.len = 0;
        fd = INVALID_SOCKET;
        op = OpNone;
        err = 0;
    }
};

typedef std::function<void(OverlapFd*)>     ConnectCallback;
typedef std::function<void(OverlapFd*)>     AcceptCallback;
typedef std::function<void(int,int)>        ReadCallback;
typedef std::function<void(int,int)>        WriteCallback;
typedef std::function<void()>               TimerCallback;
