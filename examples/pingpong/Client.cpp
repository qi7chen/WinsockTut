// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Client.h"
#include <WS2tcpip.h>
#include <functional>
#include "Common/Logging.h"
#include "Common/StringUtil.h"
#include "Common/Error.h"


enum
{
    MAX_SEND_COUNT = 5,
};

using namespace std::placeholders;


Client::Client(IOServiceBase* service)
    : recv_ctx_(NULL), service_(service), fd_(INVALID_SOCKET), sent_count_(0)
{
}

Client::~Client()
{
    Cleanup();
}

void Client::Cleanup()
{
    if (fd_ != INVALID_SOCKET)
    {
        fprintf(stderr, "%d closed\n", fd_);
        closesocket(fd_);
        fd_ = INVALID_SOCKET;
    }
    if (recv_ctx_ != NULL)
    {
        recv_ctx_->flags |= FLAG_LAZY_DELETE;
        service_->FreeOverlapCtx(recv_ctx_);
        recv_ctx_ = NULL;
    }
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
        OverlapContext* ctx = service_->AllocOverlapCtx(fd, FLAG_ASSOCIATE);
        if (ctx == NULL)
        {
            continue;
        }
        fd_ = fd;
        ctx->cb = std::bind(&Client::OnConnect, this, ctx);
        int r = service_->AsyncConnect(ctx, pinfo);
        if (r < 0)
        {
            service_->FreeOverlapCtx(ctx);
            continue;
        }
        recv_ctx_ = ctx;
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

    StartRead();
    service_->AddTimer(1000, this);
}

void Client::StartRead()
{
    recv_buf_.resize(1024);
    recv_ctx_->SetBuffer(&recv_buf_[0], recv_buf_.size());
    recv_ctx_->cb = std::bind(&Client::OnRead, this, recv_ctx_);
    service_->AsyncRead(recv_ctx_);
}

void Client::OnRead(OverlapContext* ctx)
{
    DWORD dwErr = ctx->GetStatusCode();
    if (dwErr != 0)
    {
        if (dwErr != ERROR_IO_PENDING)
        {
            LOG(ERROR) << StringPrintf("recv error %d: %s", dwErr, GetErrorMessage(dwErr));
            Cleanup();
        }
        return;
    }
    int nbytes = ctx->GetTransferredBytes();
    if (nbytes > 0)
    {
        fprintf(stdout, "%d recv %d bytes, %d\n", fd_, nbytes, sent_count_);
        recv_buf_.resize(nbytes);
    }
    // read again
    StartRead();
}

int Client::Write(const void* data, int len)
{
    DCHECK(data != NULL && len > 0);
    OverlapContext* ctx = service_->AllocOverlapCtx(fd_, 0);
    if (ctx == NULL)
    {
        return -1;
    }
    ctx->buf.buf = new char[len];
    memcpy(ctx->buf.buf, data, len);
    ctx->buf.len = len;
    ctx->cb = std::bind(&Client::OnWritten, this, ctx);
    service_->AsyncWrite(ctx);
    return 0;
}

void Client::OnWritten(OverlapContext* ctx)
{
    DWORD dwErr = ctx->GetStatusCode();
    if (dwErr == 0)
    {
        int nbytes = ctx->GetTransferredBytes();
        if (nbytes > 0)
        {
            fprintf(stdout, "%d send %d bytes, %d\n", fd_, nbytes, sent_count_);
            sent_count_++;
        }
    }
    else
    {
        LOG(WARNING) << StringPrintf("Send error: %d, %s", dwErr, GetErrorMessage(dwErr));
    }

    delete ctx->buf.buf;
    ctx->buf.buf = NULL;
    ctx->buf.len = 0;
    ctx->flags |= FLAG_LAZY_DELETE;
    service_->FreeOverlapCtx(ctx);
}

void Client::OnTimeout()
{
    if (sent_count_ < MAX_SEND_COUNT) // write again
    {
        if (!recv_buf_.empty())
        {
            Write(&recv_buf_[0], recv_buf_.size());
        }
    }
    else
    {
        Cleanup();
    }
    service_->AddTimer(1000, this);
}
