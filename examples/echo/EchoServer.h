// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include <unordered_map>
#include "Reactor/PollerBase.h"
#include "Reactor/PollEvent.h"


class EchoConn : public IPollEvent
{
public:
    explicit EchoConn(PollerBase* poller, SOCKET fd);
    ~EchoConn();

    void OnReadable();
    void OnWritable();
    void OnTimeout(){}

    void StartRead();
    void Close();

private:
    PollerBase* poller_;
    SOCKET  fd_;
    int     cap_;
    int     size_;
    char*   buf_;
};

class EchoServer : public IPollEvent
{
public:
    explicit EchoServer(PollerBase* poller);
    ~EchoServer();

    void Start(const char* host, const char* port);
    
private:
    SOCKET CreateTCPAcceptor(const char* host, const char* port);
    void Cleanup();

    void OnReadable();
    void OnWritable();
    void OnTimeout(){}

private:
    PollerBase* poller_;
    SOCKET      acceptor_;
    std::unordered_map<SOCKET, EchoConn*> connections_;
};
