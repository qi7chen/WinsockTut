// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoServer.h"
#include <functional>
#include <WS2tcpip.h>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringUtil.h"
#include "SocketOpts.h"


//////////////////////////////////////////////////////

EchoServer::EchoServer(PollerBase* poller)
    : acceptor_(INVALID_SOCKET)
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
	if (acceptor_ != INVALID_SOCKET)
	{
		poller_->RemoveFd(acceptor_);
		closesocket(acceptor_);
		acceptor_ = INVALID_SOCKET;
	}

    for (auto iter = connections_.begin(); iter != connections_.end(); ++iter)
    {
        EchoConn* conn = iter->second;
        conn->Close();
        delete conn;
    }
    connections_.clear();
}

void EchoServer::Close(SOCKET fd)
{
	LOG(INFO) << StringPrintf("sock %d closed.", fd);
	auto iter = connections_.find(fd);
	if (iter != connections_.end()) 
	{
		poller_->RemoveFd(fd);
		closesocket(fd);
		EchoConn* conn = iter->second;
		delete conn;
		connections_.erase(iter);
	}
}

void EchoServer::OnReadable()
{
    SOCKET newfd = accept(acceptor_, NULL, NULL);
    if (newfd == INVALID_SOCKET )
    {
        LOG(ERROR) << "accept " << LAST_ERROR_MSG;
        return;
    }
	LOG(INFO) << StringPrintf("sock %d accepted.", newfd);
    EchoConn* conn = new EchoConn(this, newfd);
    poller_->AddFd(newfd, conn);
    poller_->SetPollIn(newfd);
    poller_->SetPollOut(newfd);
    conn->StartRead();
    connections_[newfd] = conn; // keep in memory
}


void EchoServer::OnWritable()
{
    // do nothing here
}
