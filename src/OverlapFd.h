// Copyright (C) 2012-present prototyped.cn All rights reserved.
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

struct OverlapContext
{
    WSAOVERLAPPED   overlap;    // overlapped structure
    WSABUF          buf;        // buffer
    OperationType   op;         // operation type
    SOCKET          fd;         // socket descriptor
    int             error;      // error code
    void*			udata;      // user data
};

typedef std::function<void(int)>        ConnectCallback;
typedef std::function<void(SOCKET)>     AcceptCallback;
typedef std::function<void(int,int)>    ReadCallback;
typedef std::function<void(int,int)>    WriteCallback;
typedef std::function<void()>           TimerCallback;

struct AcceptInfo
{
    struct sockaddr_storage local_addr;
    struct sockaddr_storage remote_addr;
};
