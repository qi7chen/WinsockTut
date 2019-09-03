// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "ChatServer.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "SocketOpts.h"


using namespace std::placeholders;


ChatServer::ChatServer(IOServiceBase* service)
    : service_(service), acceptor_(INVALID_SOCKET)
{
}

ChatServer::~ChatServer()
{

}

void ChatServer::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port);
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    acceptor_ = fd;
    service_->AsyncAccept(acceptor_, std::bind(&ChatServer::OnAccept, this, _1));
}

void ChatServer::CloseSession(SOCKET fd)
{

}

void ChatServer::OnAccept(SOCKET fd)
{

}
