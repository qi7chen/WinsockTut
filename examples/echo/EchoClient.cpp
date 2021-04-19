// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
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

enum 
{
	MAX_SEND_COUNT = 5,
};

EchoClient::EchoClient(PollerBase* poller)
    :fd_(INVALID_SOCKET)
{
    poller_ = poller;
	buf_.reserve(1024);
    sent_count_ = 0;
}

EchoClient::~EchoClient()
{
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
	host_ = host;
	port_ = port;
    SOCKET fd = CreateTcpConnector(host, port); 
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
    if (nbytes < 0)
    {
        Cleanup();
    }
    else if(nbytes > 0)
    {
        fprintf(stdout, "%d recv %d bytes, %d\n", fd_, nbytes, sent_count_);
		buf_.resize(nbytes);
		memcpy(&buf_[0], buf, nbytes);
    }
}

void EchoClient::OnWritable()
{
    // connected
	if (IsSelfConnection(fd_))
	{
		Cleanup();
		Start(host_.c_str(), port_.c_str());
		return;
	}
    poller_->AddTimer(1000, this);
    poller_->ResetPollOut(fd_);
}

void EchoClient::OnTimeout()
{
	if (fd_ != INVALID_SOCKET)
	{
		SendData();
	}

    if (sent_count_ < MAX_SEND_COUNT)
    {
        poller_->AddTimer(1000, this);
    }
}

void EchoClient::SendData()
{
    const char* msg = "a quick brown fox jumps over the lazy dog";
	int len = (int)strlen(msg);
	if (sent_count_ > 0 && !buf_.empty())
	{
		msg = &buf_[0];
		len = (int)buf_.size();
	}
    sent_count_++;
    int nbytes = WriteSome(fd_, msg, len);
    if (nbytes < 0)
    {
        Cleanup();
    }
    else
    {
        fprintf(stdout, "%d send %d bytes, %d\n", fd_, nbytes, sent_count_);
		buf_.clear();
    }
}

