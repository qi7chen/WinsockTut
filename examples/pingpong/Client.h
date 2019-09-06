// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include "IOServiceBase.h"

class Client
{
public:
    explicit Client(IOServiceBase* service);
    ~Client();

    int Start(const char* host, const char* port);

private:
    int Connect(const char* host, const char* port);

    void OnConnect(OverlapContext* ctx);
    void OnRead(OverlapContext* ctx);
    void OnWritten(OverlapContext* ctx);

private:
    IOServiceBase*      service_;
    OverlapContext*     ctx_;
    std::vector<char>   recv_buf_;
};
