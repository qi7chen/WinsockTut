// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Server.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "SocketOpts.h"


using namespace std::placeholders;


Server::Server(IOServiceBase* service)
    : service_(service), ctx_(NULL)
{
}

Server::~Server()
{

}

void Server::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port);
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    OverlapContext* ctx = service_->AllocOverlapCtx();
    CHECK(ctx != NULL);
    ctx->fd = fd;
    service_->AsyncAccept(ctx, std::bind(&Server::OnAccept, this, _1));
}

void Server::CloseSession(SOCKET fd)
{

}

void Server::OnAccept(OverlapContext* ctx)
{

}
