// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include "Common/Util.h"
#include "Common/Logging.h"

using namespace std::placeholders;

enum
{
    MAX_CONN_RECVBUF = 1024,
};

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
    Connection* conn = (Connection*)malloc(sizeof(Connection) + MAX_CONN_RECVBUF);
    memset(conn, 0, sizeof(Connection) + MAX_CONN_RECVBUF);
    conn->cap = MAX_CONN_RECVBUF;
    conn->size = 0;
    connections_[newfd] = conn;
    loop_->AddEvent(newfd, EV_READABLE, std::bind(&EchoServer::OnReadable, this, _1, _2, _3));
    loop_->AddEvent(newfd, EV_WRITABLE, std::bind(&EchoServer::OnWritable, this, _1, _2, _3));
    StartRead(newfd);
}

void EchoServer::StartRead(SOCKET fd)
{
    Connection* conn = connections_[fd];
    int bytes = 0;
    while (true)
    {
        int r = recv(fd, conn->buf, conn->cap, 0);
        if (r == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << "recv: " << LAST_ERROR_MSG;
                Cleanup(fd);
            }
            else
            {
                conn->size = bytes;
            }
            break;
        }
        if (r == 0) // EOF
        {
            Cleanup(fd);
            break;
        }
        bytes += r;
    }
}

void EchoServer::OnReadable(SOCKET fd, int mask, int err)
{
    StartRead(fd);
}

void EchoServer::OnWritable(SOCKET fd, int mask, int err)
{
    Connection* conn = connections_[fd];
    if (conn == NULL)
    {
        return;
    }
    int bytes = 0;
    while(bytes < conn->size)
    {
        int remain = conn->size - bytes;
        int r = send(fd, conn->buf, remain, 0);
        if (r == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << "send: " << LAST_ERROR_MSG;
               Cleanup(fd);
            }
            break;
        }
        bytes += r;
    }
    conn->size = 0;
}

void EchoServer::Run()
{
    loop_->Run();
}
