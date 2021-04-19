// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

class Server;

class Session
{
public:
    Session(int id, Server* server, OverlapContext* ctx);
    ~Session();

    void StartRead();
    void Close();
    int Write(const void* data, int len);

private:
    void OnRead(OverlapContext* ctx);
    void OnWritten(OverlapContext* ctx);

private:
    IOServiceBase*      service_;
    Server*             server_;
    int                 id_;
    SOCKET              fd_;
    OverlapContext*     recv_ctx_;
    std::vector<char>   recv_buf_;
    int                 recv_count_;
};
