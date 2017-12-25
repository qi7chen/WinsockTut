// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include "Common/Util.h"
#include "Common/Logging.h"

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
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    acceptor_ = fd;
    loop_->AddEvent(fd, EV_READABLE, std::bind(&EchoServer::OnAccept, this, _1, _2, _3));
}

void EchoServer::Cleanup(SOCKET fd)
{
    loop_->DelEvent(fd, EV_READABLE | EV_WRITABLE);
    auto iter = connections_.find(fd);
    if (iter != connections_.end())
    {
        free(iter->second);
    }
    closesocket(fd);
}

void EchoServer::OnAccept(SOCKET fd, int mask, int err)
{
    SOCKET newfd = accept(fd, NULL, NULL);
    if (newfd == INVALID_SOCKET )
    {
        LOG(ERROR) << "accept " << LAST_ERROR_MSG;
        Cleanup(fd);
        return;
    }
    Connection* conn = (Connection*)malloc(sizeof(Connection) + 1024);
    conn->cap = 1024;
    conn->size = 0;
    connections_[fd] = conn;
    loop_->AddEvent(fd, EV_READABLE, std::bind(&EchoServer::OnReadable, this, _1, _2, _3));
    loop_->AddEvent(fd, EV_WRITABLE, std::bind(&EchoServer::OnWritable, this, _1, _2, _3));
    StartRead(fd);
}

void EchoServer::StartRead(SOCKET fd)
{
    Connection* conn = connections_[fd];
    int r = recv(fd, conn->buf, conn->cap, 0);
    if (r == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LOG(ERROR) << "recv: " << LAST_ERROR_MSG;
            Cleanup(fd);
        }
    }
}

void EchoServer::OnReadable(SOCKET fd, int mask, int err)
{
    StartRead();
}

void EchoServer::OnWritable(SOCKET fd, int mask, int err)
{
}

void EchoServer::Run()
{
    loop_->Run();
}
