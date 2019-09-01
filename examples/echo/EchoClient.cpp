// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoClient.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <functional>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringUtil.h"
#include "SocketOpts.h"

using namespace std::placeholders;

EchoClient::EchoClient(PollerBase* poller)
    :fd_(INVALID_SOCKET)
{
    poller_ = poller;
    sent_count_ = 10;
}

EchoClient::~EchoClient()
{
    Cleanup();
}

void EchoClient::Cleanup()
{
    fprintf(stderr, "%d closed\n", fd_);
    poller_->RemoveFd(fd_);
    closesocket(fd_);
    fd_ = INVALID_SOCKET;
}

void EchoClient::Start(const char* host, const char* port)
{
    SOCKET fd = Connect(host, port);
    CHECK(fd != INVALID_SOCKET);
    fd_ = fd;
    poller_->AddFd(fd, this);
    poller_->SetPollIn(fd);
    poller_->SetPollOut(fd);
}

void EchoClient::OnReadable()
{
    char buf[1024] = {};
    int nbytes = ReadSome(fd_, buf, 1024);
    if (nbytes < 1)
    {
        Cleanup();
    }
    else
    {
        fprintf(stdout, "%d recv %d bytes\n", fd_, nbytes);
    }
}

void EchoClient::OnWritable()
{
    // connected
    poller_->AddTimer(1000, this);

    poller_->ResetPollOut(fd_);
}

void EchoClient::OnTimeout()
{
    SendData();

    if (sent_count_-- > 0)
    {
        poller_->AddTimer(1000, this);
    }
}

void EchoClient::SendData()
{
    const char msg[] = "a quick brown fox jumps over the lazy dog";
    int nbytes = WriteSome(fd_, msg, (int)strlen(msg));
    if (nbytes < 0)
    {
        Cleanup();
    }
    else
    {
        fprintf(stdout, "%d send %d bytes\n", fd_, nbytes);
    }
}

SOCKET EchoClient::Connect(const char* host, const char* port)
{
    SOCKET fd = CreateTcpConnector(host, port); 
    return fd;
}
