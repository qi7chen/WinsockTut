// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Session.h"
#include "Server.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "Common/StringUtil.h"
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
        fprintf(stderr, "%d closed\n", (int)fd_);
        closesocket(fd_);
        fd_ = INVALID_SOCKET;
        int sid = id_;
        id_ = 0;
        recv_ctx_->flags |= FLAG_LAZY_DELETE;
        service_->FreeOverlapCtx(recv_ctx_);
        server_->CloseSession(sid); // will delete me
    }
}

int Session::Write(const void* data, int len)
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
    ctx->cb = std::bind(&Session::OnWritten, this, ctx);
    service_->AsyncWrite(ctx);
    return 0;
}

void Session::StartRead()
{
    recv_buf_.resize(1024);
    recv_ctx_->SetBuffer(&recv_buf_[0], (int)recv_buf_.size());
    recv_ctx_->cb = std::bind(&Session::OnRead, this, recv_ctx_);
    service_->AsyncRead(recv_ctx_);
}

void Session::OnRead(OverlapContext* ctx)
{
    DWORD dwErr = ctx->GetStatusCode();
    if (dwErr != 0)
    {
        if (dwErr != ERROR_IO_PENDING)
        {
            LOG(ERROR) << StringPrintf("recv error %d: %s", dwErr, GetErrorMessage(dwErr));
            Close();
        }
        return;
    }
    int nbytes = ctx->GetTransferredBytes();
    if (nbytes > 0)
    {
        fprintf(stdout, "%d recv %d bytes, %d\n", (int)fd_, nbytes, recv_count_);
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

void Session::OnWritten(OverlapContext* ctx)
{
    if (ctx->GetStatusCode() != 0)
    {
        LOG(WARNING) << "Send error: " << ctx->GetStatusCode();
    }
    int nbytes = ctx->GetTransferredBytes();
    if (nbytes > 0)
    {
        fprintf(stdout, "%d send %d bytes, %d\n", (int)fd_, nbytes, recv_count_);
    }

    delete ctx->buf.buf;
    ctx->buf.buf = NULL;
    ctx->buf.len = 0;
    ctx->flags |= FLAG_LAZY_DELETE;
    service_->FreeOverlapCtx(ctx);
}
