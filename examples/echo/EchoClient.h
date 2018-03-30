// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "PollerBase.h"
#include "PollEvent.h"

class EchoClient : public IPollEvent
{
public:
    explicit EchoClient(PollerBase* poller);
    ~EchoClient();

    void Start(const char* host, const char* port);


private:
    void OnReadable();
    void OnWritable();
    void OnTimeout();

    void Cleanup();
    SOCKET Connect(const char* host, const char* port);
    void SendData();

private:
    SOCKET      fd_;
    PollerBase* poller_;
    int         sent_count_;
};
