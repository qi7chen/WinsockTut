// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <stdint.h>
#include <functional>

enum
{
    FLAG_ASSOCIATE = 1,
    FLAG_CANCEL_IO = 2,
    FLAG_LAZY_DELETE = 3,
};

struct OverlapContext
{
    WSAOVERLAPPED           overlap;    // overlapped structure, must be the first field
    WSABUF                  buf;        // buffer
    SOCKET                  fd;         // socket descriptor
    int                     flags;      //
    int64_t                 udata;      // user data
    std::function<void()>   cb;         // callback 

    DWORD GetStatusCode() { return (DWORD)overlap.Internal; }

    DWORD GetTransferredBytes() { return (DWORD)overlap.InternalHigh; }

    void SetBuffer(void* buffer, int len)
    {
        buf.buf = (char*)buffer;
        buf.len = len;
    }
};

struct AcceptInfo
{
    struct sockaddr_storage local_addr; 
    char reserved1[16];
    struct sockaddr_storage remote_addr;
    char reserved2[16];
};

enum 
{
    ACCEPTEX_ADDR_LEN = sizeof(sockaddr_storage) + 16,
};
