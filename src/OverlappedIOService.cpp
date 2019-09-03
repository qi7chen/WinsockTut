// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "OverlappedIOService.h"
#include <WS2tcpip.h>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringUtil.h"
#include "SocketOpts.h"
#include "WsaExt.h"

using namespace std::placeholders;


OverlappedIOService::OverlappedIOService()
    : has_retired_(false)
{
    events_.reserve(WSA_MAXIMUM_WAIT_EVENTS);
    fds_.reserve(WSA_MAXIMUM_WAIT_EVENTS);
}

OverlappedIOService::~OverlappedIOService()
{
    CleanUp();
}

void OverlappedIOService::CleanUp()
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        OverlapContext* ctx = fds_[i];
        FreeOverlapFd(ctx);
        delete ctx;
    }
    fds_.clear();
    events_.clear();
}

int OverlappedIOService::CancelFd(SOCKET fd)
{
    OverlapContext* ctx = GetOrCreateOverlapContext(fd, false);
    if (ctx != NULL)
    {
        FreeOverlapFd(ctx);
        has_retired_ = true;
    }
    return 0;
}

OverlapContext* OverlappedIOService::GetOrCreateOverlapContext(SOCKET fd, bool will_create)
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        OverlapContext* ctx = fds_[i];
        if (ctx->fd == fd) 
        {
            return ctx;
        }
    }
    if (will_create)
    {
        WSAEVENT hEvent = WSACreateEvent();
        if (hEvent == WSA_INVALID_EVENT)
        {
            LOG(ERROR) << "AllocOverlapFd: WSACreateEvent " << LAST_ERROR_MSG;
            return NULL;
        }
        OverlapContext* ctx = new OverlapContext;
        ctx->fd = fd;
        ctx->overlap.hEvent = hEvent;
        ctx->fd = fd;
        fds_.push_back(ctx);
        return ctx;
    }
    return NULL;
}

void OverlappedIOService::FreeOverlapFd(OverlapContext* ctx)
{
    WSACloseEvent(ctx->overlap.hEvent);
    closesocket(ctx->fd);
    ctx->fd = INVALID_SOCKET;
    ctx->overlap.hEvent = WSA_INVALID_EVENT;
}

OverlapContext* OverlappedIOService::FindContextEntryByEvent(WSAEVENT hEvent)
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        OverlapContext* ctx = fds_[i];
        if (ctx->overlap.hEvent == hEvent)
        {
            return ctx;
        }
    }
    return NULL;
}

void OverlappedIOService::RemoveRetired()
{
    if (has_retired_)
    {
        for (auto iter = fds_.begin(); iter != fds_.end(); ++iter)
        {
            OverlapContext* ctx = *iter;
            if (ctx->fd == INVALID_SOCKET)
            {
                delete ctx;
                iter = fds_.erase(iter);
            }
        }
        has_retired_ = false;
    }
}

int OverlappedIOService::AsyncConnect(SOCKET fd, const addrinfo* pinfo, ConnectCallback cb)
{
    // ConnectEx need previously bound socket.
    if (BindAnyAddr(fd, pinfo->ai_family) != 0)
    {
        return -1;
    }
    OverlapContext* ctx = GetOrCreateOverlapContext(fd, true);
    if (ctx == NULL)
    {
        return -1;
    }
    int r = WsaExt::ConnectEx(fd, (const struct sockaddr*)pinfo->ai_addr, (int)pinfo->ai_addrlen,
        nullptr, 0, nullptr, &ctx->overlap);
    if (r != TRUE)
    {
        if (GetLastError() != WSA_IO_PENDING)
        {
            LOG(ERROR) << "AsyncConnect: " << LAST_ERROR_MSG;
            FreeOverlapFd(ctx);
            return false;
        }
    }
    else
    {
        UpdateConnectCtx(fd);
    }
    ctx->op = OpConnect;
    ctx->udata = new ConnectCallback(cb);
    return 0; // succeed
}

int OverlappedIOService::AsyncAccept(SOCKET acceptor, AcceptCallback cb)
{
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
        return -1;
    }
    OverlapContext* ctx = GetOrCreateOverlapContext(fd, true);
    if (ctx == NULL)
    {
        closesocket(fd);
        return -1;
    }
    OverlapContext* acceptorCtx = GetOrCreateOverlapContext(acceptor, true);
    if (ctx == NULL)
    {
        return -1;
    }
    if (acceptorCtx->buf.buf == NULL)
    {
        acceptorCtx->buf.buf = (char*)new AcceptInfo;
        acceptorCtx->buf.len = sizeof(AcceptInfo);
    }
    else
    {
        memset(acceptorCtx->buf.buf, 0, acceptorCtx->buf.len);
    }
    int fOK = WsaExt::AcceptEx(acceptor, fd, acceptorCtx->buf.buf, 0, sizeof(sockaddr_storage), 
        sizeof(sockaddr_storage), NULL, &acceptorCtx->overlap);
    if (!fOK)
    {
        int r = WSAGetLastError();
        if (r != ERROR_IO_PENDING)
        {
            LOG(ERROR) << StringPrintf("AcceptEx(): %s\n", LAST_ERROR_MSG);
            return r;
        }
    }
    return 0;
}

int OverlappedIOService::AsyncRead(void* buf, int size, ReadCallback cb)
{
    return 0;
}

int OverlappedIOService::AsyncWrite(const void* buf, int size, WriteCallback cb)
{
    return 0;
}

int OverlappedIOService::Run(int timeout)
{
    int nready = 0;
    events_.clear();
    for (size_t i = 0; i < fds_.size(); i++)
    {
        OverlapContext* ctx = fds_[i];
        if (ctx->fd != INVALID_SOCKET && ctx->overlap.hEvent != WSA_INVALID_EVENT)
        {
            events_.push_back(ctx->overlap.hEvent);
        }
    }
    if (!events_.empty())
    {
        nready = WSAWaitForMultipleEvents((DWORD)events_.size(), &events_[0], FALSE, timeout, FALSE);
        if (nready == WSA_WAIT_FAILED)
        {
            LOG(ERROR) << "Poll: WSAWaitForMultipleEvents, " << LAST_ERROR_MSG;
            nready = -1;
        }
        else if (nready == WSA_WAIT_TIMEOUT)
        {
            nready = -1;
        }
    }
    else
    {
        // nothing to do, sleep a while
        if (timeout > 0)
        {
            Sleep(timeout / 2);
        }
    }
    UpdateTimer();
    if (nready >= 0 && !events_.empty())
    {
        int index = nready - WSA_WAIT_EVENT_0;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            LOG(ERROR) << "Poll: wait events index out of range: " << index;
            return -1;
        }
        WSAEVENT hEvent = events_[index];
        OverlapContext* ctx = FindContextEntryByEvent(hEvent);
        if (ctx == NULL)
        {
            LOG(ERROR) << "Poll: overlap context not found";
            return -1;
        }
        DWORD dwBytes = 0;
        DWORD dwFlags = 0;
        ctx->error = 0;
        if (!WSAGetOverlappedResult(ctx->fd, &ctx->overlap, &dwBytes, 1, &dwFlags))
        {
            ctx->error = WSAGetLastError();
        }
        DispatchEvent(ctx);
    }
    RemoveRetired();
    return 0;
}

void OverlappedIOService::DispatchEvent(OverlapContext* ctx)
{
    switch(ctx->op)
    {
    case OpConnect:
        {
            ConnectCallback* callback = (ConnectCallback*)ctx->udata;
            (*callback)(ctx->error);
            delete ctx->udata;
        }
        break;

    case OpAccept:
        break;

    case OpRead:
        break;

    case OpWrite:
        break;

    case OpClose:
        break;

    default:
        return;
    }
}
