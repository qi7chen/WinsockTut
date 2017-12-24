// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include "Common/Util.h"

using namespace std::placeholders;

EchoServer::EchoServer(IOMode mode)
{
    loop_ = new EventLoop(mode);
    acceptor_ = INVALID_SOCKET;
}

EchoServer::~EchoServer()
{
    closesocket(acceptor_);
    acceptor_ = INVALID_SOCKET;
    delete loop_;
}

void EchoServer::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port, true);
    if (fd == INVALID_SOCKET)
    {
        abort();
    }
    acceptor_ = fd;
    loop_->AddEvent(fd, EV_READABLE, std::bind(&EchoServer::OnAccept, this, _1, _2, _3));
}

void EchoServer::OnAccept(EventLoop* loop, SOCKET fd, int mask)
{
    SOCKET newfd = accept(fd, NULL, NULL);
}

void EchoServer::Run()
{
    loop_->Run();
}
