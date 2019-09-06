// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Server.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "Common/StringUtil.h"
#include "SocketOpts.h"
#include "Session.h"
#include "WsaExt.h"

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
        ctx_->flags |= FLAG_LAZY_DELETE;
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
    OverlapContext* ctx = service_->AllocOverlapCtx(fd, FLAG_ASSOCIATE);
    CHECK(ctx != NULL);
    acceptor_ = fd;
    ctx_ = ctx;
    StartAccept();
}

void Server::StartAccept()
{
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
    ctx_->cb = std::bind(&Server::OnAccept, this, ctx_);
    service_->AsyncAccept(ctx_);
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
    OverlapContext* newctx = service_->AllocOverlapCtx(newfd, FLAG_ASSOCIATE);
    if (newctx == NULL)
    {
        closesocket(newfd);
        return;
    }

    int sid = counter_++;
    Session* session = new Session(sid, this, newctx);
    sessions_[sid] = session;
    session->StartRead();

    AcceptInfo* info = (AcceptInfo*)ctx->buf.buf;
    struct sockaddr* remote_addr = NULL;
    struct sockaddr* local_addr = NULL;
    int local_len = 0;
    int remote_len = 0;
    WsaExt::GetAcceptExSockaddrs(info, 0, ACCEPTEX_ADDR_LEN, ACCEPTEX_ADDR_LEN,
        &local_addr, &local_len, &remote_addr, &remote_len);

    char buffer[40] = {};
    inet_ntop(remote_addr->sa_family, remote_addr->sa_data, buffer, sizeof(buffer));
    LOG(INFO) << StringPrintf("%d accepted from %s", newfd, buffer);

    StartAccept();
}
