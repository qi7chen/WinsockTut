// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include <WS2tcpip.h>
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
    int nbytes = ReadSome(fd_, buf_, cap_);
    if (nbytes <= 0)
    {
        Close(); // EOF or error;
        return;
    }
    size_ = nbytes;
    fprintf(stdout, "%d recv %d bytes\n", fd_, nbytes);

    // echo back
    nbytes = WriteSome(fd_, buf_, size_);
    if (nbytes < 0)
    {
        Close();
        return ;
    }
    fprintf(stdout, "%d send %d bytes\n", fd_, nbytes);
}

void EchoConn::OnReadable()
{
    StartRead();
}

void EchoConn::OnWritable()
{
    // writable
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


// create acceptor socket
SOCKET EchoServer::CreateTCPAcceptor(const char* host, const char* port)
{
    SOCKET fd = INVALID_SOCKET;
    RangeTCPAddrList(host, port, [&](const addrinfo* pinfo) -> bool
    {
        fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
            return false;
        }
        int err = bind(fd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (err == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("%s bind(): %s\n", pinfo->ai_addr, LAST_ERROR_MSG);
            closesocket(fd);
            fd = INVALID_SOCKET;
            return false;
        }
        // set to non-blocking mode
        if (SetNonblock(fd, true) == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("ioctlsocket(): %s\n", LAST_ERROR_MSG);
            closesocket(fd);
            fd = INVALID_SOCKET;
            return false;
        }
        err = listen(fd, SOMAXCONN);
        if (err == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("listen(): %s\n", LAST_ERROR_MSG);
            closesocket(fd);
            fd = INVALID_SOCKET;
            return false;
        }
        return true;
    });

    return fd;
}