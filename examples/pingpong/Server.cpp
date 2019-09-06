// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Server.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "SocketOpts.h"
#include "Session.h"


using namespace std::placeholders;


Server::Server(IOServiceBase* service)
    : service_(service), 
    ctx_(NULL), 
    acceptor_(INVALID_SOCKET),
    counter_(2001)
{
}

Server::~Server()
{
    Cleanup();
}

void Server::Cleanup()
{
    if (acceptor_ != INVALID_SOCKET)
    {
        fprintf(stderr, "%d closed\n", acceptor_);
        closesocket(acceptor_);
        acceptor_ = INVALID_SOCKET;
    }
    if (ctx_ != NULL)
    {
        delete ctx_->buf.buf;
        ctx_->buf.buf = NULL;
        service_->FreeOverlapCtx(ctx_);
        ctx_ = NULL;
    }
}

void Server::CloseSession(int sid)
{
    auto iter = sessions_.find(sid);
    if (iter != sessions_.end())
    {
        Session* session = iter->second;
        delete session;
        sessions_.erase(iter);
    }
}

void Server::Start(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port);
    CHECK(fd != INVALID_SOCKET) << LAST_ERROR_MSG;
    OverlapContext* ctx = service_->AllocOverlapCtx();
    CHECK(ctx != NULL);
    ctx->fd = fd;
    acceptor_ = fd;
    ctx_ = ctx;
    StartAccept();
}

void Server::StartAccept()
{
    ctx_->op = OpAccept;
    if (ctx_->buf.buf == NULL)
    {
        AcceptInfo* pinfo = new AcceptInfo();
        ctx_->buf.buf = (char*)pinfo;
        ctx_->buf.len = sizeof(AcceptInfo);
    }
    else
    {
        memset(ctx_->buf.buf, 0, ctx_->buf.len);
    }
    service_->AsyncAccept(ctx_, std::bind(&Server::OnAccept, this, _1));
}

void Server::OnAccept(OverlapContext* ctx)
{
    SOCKET newfd = (SOCKET)ctx->udata;
    ctx->udata = 0;
    if (ctx->GetStatusCode() != 0)
    {
        LOG(ERROR) << "accept failed: " << ctx->GetStatusCode();
        closesocket(newfd);
        return;
    }
    OverlapContext* newctx = service_->AllocOverlapCtx();
    if (newctx == NULL)
    {
        closesocket(newfd);
        return;
    }
    newctx->fd = newfd;
    AcceptInfo* info = (AcceptInfo*)ctx->buf.buf;
    int sid = counter_++;
    Session* session = new Session(sid, this, newctx);
    sessions_[sid] = session;
    session->StartRead();
    StartAccept();
}
