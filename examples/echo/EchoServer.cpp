// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "SocketOpts.h"

using namespace std::placeholders;

enum
{
    MAX_CONN_RECVBUF = 1024,
};

EchoConn::EchoConn(PollerBase* poller, SOCKET fd)
{
    fd_ = fd;
    poller_ = poller;
    cap_ = MAX_CONN_RECVBUF;
    size_ = 0;
    buf_ = new char[cap_];
}

EchoConn::~EchoConn()
{
    Close();
}

void EchoConn::Close()
{
    poller_->RemoveFd(fd_);
    closesocket(fd_);
    fd_ = INVALID_SOCKET;
    cap_ = 0;
    size_ = 0;
    if (buf_ != nullptr)
    {
        delete buf_;
        buf_ = nullptr;
    }
}

void EchoConn::StartRead()
{
    while (true)
    {
        int r = recv(fd_, buf_, cap_, 0);
        if (r > 0)
        {
            size_ += r;
        }
        else
        {
            if (r == SOCKET_ERROR) 
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    Close(); // EOF or error;
                    return;
                }
            }
            break;
        }
    }
    LOG(ERROR) << StringPrintf("socket %d recv %d bytes", fd_, size_);
}

void EchoConn::OnReadable()
{
    StartRead();
}

void EchoConn::OnWritable()
{
    int nbytes = 0;
    while(nbytes < size_)
    {
        int remain = size_ - nbytes;
        int r = send(fd_, buf_ + nbytes, remain, 0);
        if (r == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << "send: " << LAST_ERROR_MSG;
                Close();
            }
            break;
        }
        nbytes += r;
    }
    size_ = 0;
    LOG(ERROR) << StringPrintf("socket %d recv %d bytes", fd_, nbytes);
}

//////////////////////////////////////////////////////

EchoServer::EchoServer(PollerBase* poller)
    :acceptor_(INVALID_SOCKET)
{
    poller_ = poller;
}

EchoServer::~EchoServer()
{
    Cleanup();
}

void EchoServer::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port);
    SetNonblock(fd, true);
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    acceptor_ = fd;
    poller_->AddFd(acceptor_, this);
    poller_->SetPollIn(fd);
}

void EchoServer::Cleanup()
{
    poller_->RemoveFd(acceptor_);
    closesocket(acceptor_);
    acceptor_ = INVALID_SOCKET;

    for (auto iter = connections_.begin(); iter != connections_.end(); ++iter)
    {
        EchoConn* conn = iter->second;
        conn->Close();
        delete conn;
    }
    connections_.clear();
}

void EchoServer::OnReadable()
{
    SOCKET newfd = accept(acceptor_, NULL, NULL);
    if (newfd == INVALID_SOCKET )
    {
        LOG(ERROR) << "accept " << LAST_ERROR_MSG;
        return;
    }
    EchoConn* conn = new EchoConn(poller_, newfd);
    poller_->AddFd(newfd, conn);
    poller_->SetPollIn(newfd);
    poller_->SetPollOut(newfd);
    conn->StartRead();
    connections_[newfd] = conn;
}


void EchoServer::OnWritable()
{
    // do nothing here
}
