// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Client.h"
#include <WS2tcpip.h>
#include <functional>
#include "Common/Logging.h"
#include "Common/StringUtil.h"

using namespace std::placeholders;

Client::Client(IOServiceBase* service)
    : ctx_(NULL), service_(service)
{
}

Client::~Client()
{
}

int Client::Start(const char* host, const char* port)
{
    return Connect(host, port);
}

int Client::Connect(const char* host, const char* port)
{
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host, port, &hints, &aiList);
    if (err != 0)
    {
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return -1;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        SOCKET fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            continue;
        }
        OverlapContext* ctx = service_->AllocOverlapCtx();
        if (ctx == NULL)
        {
            continue;
        }
        ctx->fd = fd;
        int r = service_->AsyncConnect(ctx, pinfo, std::bind(&Client::OnConnect, this, _1));
        if (r < 0)
        {
            service_->FreeOverlapCtx(ctx);
            continue;
        }
        ctx_ = ctx;
        break;
    }
    freeaddrinfo(aiList);
    return 0;
}

void Client::OnConnect(OverlapContext* ctx)
{
    if (ctx->GetStatusCode() != 0)
    {
        LOG(ERROR) << "Conenct error: " << ctx->GetStatusCode();
        service_->FreeOverlapCtx(ctx);
        return;
    }
    recv_buf_.resize(1024);
    ctx->SetBuffer(&recv_buf_[0], recv_buf_.size());
    service_->AsyncRead(ctx, std::bind(&Client::OnConnect, this, _1));
}

void Client::OnRead(OverlapContext* ctx)
{
    service_->AsyncRead(ctx, std::bind(&Client::OnConnect, this, _1));
}

void Client::OnWritten(OverlapContext* ctx)
{

}
