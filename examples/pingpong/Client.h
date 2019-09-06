// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include "PollEvent.h"
#include "IOServiceBase.h"

class Client : public ITimerEvent
{
public:
    explicit Client(IOServiceBase* service);
    ~Client();

    int Start(const char* host, const char* port);

private:
    void StartRead();
    void Cleanup();
    int Connect(const char* host, const char* port);
    int Write(const void* data, int len);

    void OnConnect(OverlapContext* ctx);
    void OnRead(OverlapContext* ctx);
    void OnWritten(OverlapContext* ctx);
    void OnTimeout();

private:
    IOServiceBase*      service_;
    OverlapContext*     ctx_;
    SOCKET              fd_;
    int                 sent_count_;
    std::vector<char>   recv_buf_;
};
