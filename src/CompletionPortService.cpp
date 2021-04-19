// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "CompletionPortService.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "Common/StringUtil.h"
#include "SocketOpts.h"
#include "WsaExt.h"


CompletionPortService::CompletionPortService()
{
    HANDLE handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    CHECK(handle != NULL) << LAST_ERROR_MSG;
    completion_port_ = handle;
}

CompletionPortService::~CompletionPortService()
{
    if (completion_port_ != NULL)
    {
        CloseHandle(completion_port_);
        completion_port_ = NULL;
    }
}

OverlapContext* CompletionPortService::AllocOverlapCtx(SOCKET fd, int flags)
{
    OverlapContext* ctx = new OverlapContext();
    ctx->fd = fd;
    if (flags & FLAG_ASSOCIATE)
    {
        HANDLE handle = CreateIoCompletionPort((HANDLE)fd, completion_port_, (ULONG_PTR)fd, 0);
        if (handle != completion_port_)
        {
            LOG(ERROR) << "CreateIoCompletionPort: " << LAST_ERROR_MSG;
            FreeOverlapCtx(ctx);
            return NULL;
        }
    }
    return ctx;
}

void CompletionPortService::FreeOverlapCtx(OverlapContext* ctx)
{
    if (ctx->flags & FLAG_CANCEL_IO)
    {
        CancelIoEx((HANDLE)ctx->fd, NULL);
    }
    ctx->fd = INVALID_SOCKET;
    if (!(ctx->flags & FLAG_LAZY_DELETE))
    {
        delete ctx;
    }
}

int CompletionPortService::AsyncConnect(OverlapContext* ctx, const addrinfo* pinfo)
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
        nullptr, 0, nullptr, (WSAOVERLAPPED*)ctx);
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

int CompletionPortService::AsyncAccept(OverlapContext* ctx)
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
        NULL, (WSAOVERLAPPED*)ctx);
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

int CompletionPortService::AsyncRead(OverlapContext* ctx)
{
    DWORD dwFlags = 0;
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int r = WSARecv(ctx->fd, &ctx->buf, 1, NULL, &dwFlags, (WSAOVERLAPPED*)ctx, NULL);
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

int CompletionPortService::AsyncWrite(OverlapContext* ctx)
{
    ctx->overlap.Internal = 0;
    ctx->overlap.InternalHigh = 0;
    int r = WSASend(ctx->fd, &ctx->buf, 1, NULL, 0, (WSAOVERLAPPED*)ctx, NULL);
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

int CompletionPortService::Run(int timeout)
{
    DWORD dwBytes = 0;
    ULONG_PTR completionKey = 0;
    WSAOVERLAPPED* pOverlap = NULL;
    UpdateTimer();
    int fOK = GetQueuedCompletionStatus(completion_port_, &dwBytes, (ULONG_PTR*)&completionKey, &pOverlap, timeout);
    if (!fOK)
    {
        DWORD dwErr = GetLastError();
        if (pOverlap != NULL)
        {
            LOG(ERROR) << "GetQueuedCompletionStatus: " << LAST_ERROR_MSG;
            return -1;
        }
        else
        {
            if (dwErr != WAIT_TIMEOUT)
            {
                LOG(ERROR) << "GetQueuedCompletionStatus: " << LAST_ERROR_MSG;
                return -1;
            }
        }
    }
    OverlapContext* pContext = (OverlapContext*)pOverlap;
    if (pContext != NULL)
    {
        if (pContext->cb)
        {
            pContext->cb(); // dispatch event
        }
        if (pContext->flags & FLAG_LAZY_DELETE)
        {
            delete pContext;
        }
    }
    return 0;
}
