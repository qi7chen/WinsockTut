// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <stdint.h>
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
    int64_t			udata;      // user data
    std::function<void()> cb;   // callback 
};

typedef std::function<void(OverlapContext*)>    OverlapCallback;

struct AcceptInfo
{
    struct sockaddr_storage local_addr;
    struct sockaddr_storage remote_addr;
};
