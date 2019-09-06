// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Session.h"
#include "Server.h"
#include "Common/Logging.h"
#include <functional>

using namespace std::placeholders;

Session::Session(int id, Server* server, OverlapContext* ctx)
    : server_(server), 
    service_(server->GetIOService()),
    recv_ctx_(ctx),
    id_(id),
    fd_(ctx->fd),
    recv_count_(0)
{
}

Session::~Session()
{
    Close();
}

void Session::Close()
{
    if (id_ > 0)
    {
        fprintf(stderr, "%d closed\n", fd_);
        closesocket(fd_);
        fd_ = INVALID_SOCKET;
        int sid = id_;
        id_ = 0;
        service_->FreeOverlapCtx(recv_ctx_);
        server_->CloseSession(sid); // will delete me
    }
}

int Session::Write(const void* data, int len)
{
    DCHECK(data != NULL && len > 0);
    OverlapContext* ctx = service_->AllocOverlapCtx();
    if (ctx == NULL)
    {
        return -1;
    }
    ctx->op = OpWrite;
    ctx->fd = fd_;
    ctx->buf.buf = new char[len];
    memcpy(ctx->buf.buf, data, len);
    ctx->buf.len = len;
    service_->AsyncWrite(ctx, std::bind(&Session::OnWritten, this, _1));
    return 0;
}

void Session::StartRead()
{
    recv_buf_.resize(1024);
    recv_ctx_->op = OpRead;
    recv_ctx_->SetBuffer(&recv_buf_[0], recv_buf_.size());
    service_->AsyncRead(recv_ctx_, std::bind(&Session::OnRead, this, _1));
}

void Session::OnRead(OverlapContext* ctx)
{
    if (ctx->GetStatusCode() == 0)
    {
        int nbytes = ctx->GetTransferredBytes();
        if (nbytes > 0)
        {
            fprintf(stdout, "%d recv %d bytes, %d\n", fd_, nbytes, recv_count_);
            recv_buf_.resize(nbytes);
            recv_count_++;
            Write(&recv_buf_[0], nbytes);
            StartRead();
        }
        else // EOF
        {
            Close();
        }
    }
    else
    {
        Close();
    }
}

void Session::OnWritten(OverlapContext* ctx)
{
    if (ctx->GetStatusCode() != 0)
    {
        LOG(WARNING) << "Send error: " << ctx->GetStatusCode();
    }
    int nbytes = ctx->GetTransferredBytes();
    if (nbytes > 0)
    {
        fprintf(stdout, "%d send %d bytes, %d\n", fd_, nbytes, recv_count_);
    }

    delete ctx->buf.buf;
    ctx->buf.buf = NULL;
    ctx->buf.len = 0;
    service_->FreeOverlapCtx(ctx);
}
