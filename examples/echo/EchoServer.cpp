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
    :acceptor_(INVALID_SOCKET)
{
    loop_ = new EventLoop(mode);
}

EchoServer::~EchoServer()
{
    closesocket(acceptor_);
    acceptor_ = INVALID_SOCKET;
    delete loop_;
    for (auto iter = connections_.begin(); iter != connections_.end(); ++iter)
    {
        closesocket(iter->first);
        free(iter->second);
    }
}

void EchoServer::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port);
    SetNonblock(fd, true);
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    acceptor_ = fd;
    loop_->AddEvent(fd, std::bind(&EchoServer::HandleEvent, this, _1, _2, _3));
}

void EchoServer::Cleanup(SOCKET fd)
{
    closesocket(fd);
    auto iter = connections_.find(fd);
    if (iter != connections_.end())
    {
        free(iter->second);
        connections_.erase(iter);
    }
    loop_->DelEvent(fd);
    LOG(INFO) << "socket " << fd << " closed";
}


void EchoServer::HandleEvent(SOCKET fd, int ev, int err)
{
    if (err != 0)
    {
        LOG(ERROR) << GetErrorMessage(err);
        Cleanup(fd);
        return;
    }
    if (ev & EV_READABLE)
    {
        if (fd == acceptor_)
        {
            OnAccept(fd);
        }
        else
        {
            StartRead(fd);
        }
    }
    if (ev & EV_WRITABLE)
    {
        OnWritable(fd);
    }
}

void EchoServer::OnAccept(SOCKET fd)
{
    SOCKET newfd = accept(fd, NULL, NULL);
    if (newfd == INVALID_SOCKET )
    {
        LOG(ERROR) << "accept " << LAST_ERROR_MSG;
        return;
    }
    
    int r = loop_->AddEvent(newfd, std::bind(&EchoServer::HandleEvent, this, _1, _2, _3));
    if (r < 0)
    {
        LOG(ERROR) << "OnAccept: events full";
        closesocket(newfd);
        return;
    }
    LOG(INFO) << "socket " << newfd << " accepted";
    Connection* conn = (Connection*)malloc(sizeof(Connection) + MAX_CONN_RECVBUF);
    memset(conn, 0, sizeof(Connection) + MAX_CONN_RECVBUF);
    conn->cap = MAX_CONN_RECVBUF;
    conn->size = 0;
    connections_[newfd] = conn;
}

void EchoServer::StartRead(SOCKET fd)
{
    int r = 0;
    Connection* conn = connections_[fd];
    while (true)
    {
        r = recv(fd, conn->buf, conn->cap, 0);
        if (r > 0)
        {
            conn->size += r;
        }
        else
        {
            break;
        }
    }
    if (r == SOCKET_ERROR) 
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            return;
        }
    }
    Cleanup(fd); // EOF or error
}


void EchoServer::OnWritable(SOCKET fd)
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
