// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include <unordered_map>
#include "PollerBase.h"
#include "PollEvent.h"
#include "EchoConn.h"


class EchoServer : public IPollEvent
{
public:
    explicit EchoServer(PollerBase* poller);
    ~EchoServer();

    void Start(const char* host, const char* port);
	void CloseSession(SOCKET fd);

	PollerBase* Poller() { return poller_;  }
    
private:
    void Cleanup();

    void OnReadable();
    void OnWritable();
    void OnTimeout(){}

private:
    PollerBase* poller_;
    SOCKET      acceptor_;
    std::unordered_map<SOCKET, EchoConn*> connections_;
};
