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

    fds_.clear();
    events_.clear();
}

OverlapContext* OverlappedIOService::AllocOverlapCtx(SOCKET fd, int flags)
{
    if (fds_.size() >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG(ERROR) << "WSA_MAXIMUM_WAIT_EVENTS limit";
        return NULL;
    }
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        LOG(ERROR) << "AllocOverlapContext: WSACreateEvent " << LAST_ERROR_MSG;
        return NULL;
    }
    OverlapContext* ctx = new OverlapContext();
    ctx->fd = fd;
    ctx->overlap.hEvent = hEvent;
    fds_[hEvent] = ctx;
    return ctx;
}

void OverlappedIOService::FreeOverlapCtx(OverlapContext* ctx)
{
    WSAEVENT hEvent = ctx->overlap.hEvent;
    WSACloseEvent(hEvent);
    ctx->fd = INVALID_SOCKET;
    ctx->overlap.hEvent = WSA_INVALID_EVENT;
    fds_.erase(hEvent);
    delete ctx;
}

int OverlappedIOService::AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo)
{
    // ConnectEx need previously bound socket.
    int r = BindAnyAddr(ctx->fd, pinfo->ai_family);
    if (r != 0)
    {
        return r;
    }
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int fOK = WsaExt::ConnectEx(ctx->fd, (const struct sockaddr*)pinfo->ai_addr, (int)pinfo->ai_addrlen,
        nullptr, 0, nullptr, &ctx->overlap);
    if (!fOK)
    {
        if (GetLastError() != WSA_IO_PENDING)
        {
            LOG(ERROR) << "ConnectEx: " << LAST_ERROR_MSG;
            return -1;
        }
    }
    else
    {
        UpdateConnectCtx(ctx->fd);
    }
    return 0; // succeed
}

int OverlappedIOService::AsyncAccept(OverlapContext* ctx)
{
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
        return -1;
    }
    ctx->udata = fd;
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int fOK = WsaExt::AcceptEx(ctx->fd, fd, ctx->buf.buf, 0, ACCEPTEX_ADDR_LEN, ACCEPTEX_ADDR_LEN,
        NULL, &ctx->overlap);
    if (!fOK)
    {
        int r = WSAGetLastError();
        if (r != WSA_IO_PENDING)
        {
            closesocket(fd);
            LOG(ERROR) << StringPrintf("AcceptEx(): %s\n", LAST_ERROR_MSG);
            return r;
        }
    }
    return 0;
}

int OverlappedIOService::AsyncRead(OverlapContext* ctx)
{
    DWORD dwFlags = 0;
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int r = WSARecv(ctx->fd, &ctx->buf, 1, NULL, &dwFlags, &ctx->overlap, NULL);
    if (r == SOCKET_ERROR)
    {
        int r = WSAGetLastError();
        if (r != WSA_IO_PENDING)
        {
            LOG(ERROR) << StringPrintf("WSARecv(): %s\n", LAST_ERROR_MSG);
            return r;
        }
    }
    return 0;
}

int OverlappedIOService::AsyncWrite(OverlapContext* ctx)
{
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int r = WSASend(ctx->fd, &ctx->buf, 1, NULL, 0, &ctx->overlap, NULL);
    if (r == SOCKET_ERROR)
    {
        int r = WSAGetLastError();
        if (r != WSA_IO_PENDING)
        {
            LOG(ERROR) << StringPrintf("WSASend(): %s\n", LAST_ERROR_MSG);
            return r;
        }
    }
    return 0;
}

int OverlappedIOService::Run(int timeout)
{
    int nready = 0;
    events_.clear();
    for (auto iter = fds_.begin(); iter != fds_.end(); ++iter)
    {
        events_.push_back(iter->first);
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
        auto iter = fds_.find(hEvent);
        if (iter == fds_.end())
        {
            //LOG(ERROR) << "Run: overlap context not found";
            return -1;
        }
        OverlapContext* pContext = iter->second;
        DWORD dwBytes = 0;
        DWORD dwFlags = 0;
        if (!WSAGetOverlappedResult(pContext->fd, &pContext->overlap, &dwBytes, 1, &dwFlags))
        {
            LOG(ERROR) << "WSAGetOverlappedResult: " << LAST_ERROR_MSG;
        }
        if (pContext && pContext->cb)
        {
            pContext->cb(); // dispatch event
        }
    }
    return 0;
}
