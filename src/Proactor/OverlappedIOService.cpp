// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "OverlappedIOService.h"
#include <WS2tcpip.h>
#include "Common/Any.h"
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "SocketOpts.h"
#include "WsaExt.h"


//////////////////////////////////////////////////////////////////////////
OverlappedIOService::OverlappedIOService()
{
    event_socks_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
    sock_handles_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
}

OverlappedIOService::~OverlappedIOService()
{
    CleanUp();
}

void OverlappedIOService::CleanUp()
{
    for (auto iter = sock_handles_.begin(); iter != sock_handles_.end(); ++iter)
    {
        FreeOverlapFd(iter->second);
    }
    sock_handles_.clear();
    event_socks_.clear();
}

OverlapFd* OverlappedIOService::AllocOverlapFd(SOCKET fd)
{
    auto iter = sock_handles_.find(fd);
    if (iter != sock_handles_.end())
    {
        return iter->second;
    }

    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        LOG(ERROR) << "WSACreateEvent: " << LAST_ERROR_MSG;
        return nullptr;
    }
    OverlapFd* data = new OverlapFd;
    data->fd = fd;
    data->overlap.hEvent = hEvent;
    sock_handles_[fd] = data;
    event_socks_[hEvent] = fd;
    return data;
}

void OverlappedIOService::FreeOverlapFd(OverlapFd* data)
{
    sock_handles_.erase(data->fd);
    event_socks_.erase(data->overlap.hEvent);
    CloseHandle(data->overlap.hEvent);
    closesocket(data->fd);
    delete data;
}

OverlapFd* OverlappedIOService::GetOverlapFdByEvent(WSAEVENT hEvent)
{
    auto iter = event_socks_.find(hEvent);
    if (iter != event_socks_.end())
    {
        auto it = sock_handles_.find(iter->second);
        if (it != sock_handles_.end())
        {
            return it->second;
        }
    }
    return nullptr;
}

int OverlappedIOService::AsyncConnect(const char* addr, const char* port, ConnectCallback cb)
{
    return RangeTCPAddrList(addr, port, [=](const addrinfo* pinfo) -> bool
    {
        // fd has the overlapped attribute as a default.
        SOCKET fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
            return false; // try next addrinfo
        }

        // ConnectEx need previously bound socket.
        if (BindAnyAddr(fd, pinfo->ai_family) != 0)
        {
            closesocket(fd);
            return false;
        }

        OverlapFd* data = AllocOverlapFd(fd);
        CHECK(data != nullptr) << "AllocHandleData";

        int r = WsaExt::ConnectEx(fd, (const struct sockaddr*)pinfo->ai_addr, (int)pinfo->ai_addrlen, 
            nullptr, 0, nullptr, (OVERLAPPED*)data);
        if (r != TRUE)
        {
            if (GetLastError() != WSA_IO_PENDING)
            {
                LOG(ERROR) << "ConenctEx: " << LAST_ERROR_MSG;
                FreeOverlapFd(data);
                return false;
            }
        }
        else
        {
            // enable previously set properties or options
            r = setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
            if (r != 0)
            {
                FreeOverlapFd(data);
                delete data;
            }
        }
        data->op = OpConnect;
        data->ctx = cb;
        return true; // succeed
    });
}

int OverlappedIOService::AsyncListen(OverlapFd* fd, const char* addr, const char* port, AcceptCallback cb)
{
    return 0;
}

int OverlappedIOService::AsyncRead(OverlapFd* fd, void* buf, int size, ReadCallback cb)
{
    return 0;
}

int OverlappedIOService::AsyncWrite(OverlapFd* fd, void* buf, int size, WriteCallback cb)
{
    return 0;
}

int OverlappedIOService::Poll(int timeout)
{
    if (event_socks_.empty())
    {
        UpdateTimer();
        Sleep(timeout);
        return 0;
    }
    int count = 0;
    for (auto iter = event_socks_.begin(); iter != event_socks_.end(); ++iter)
    {
        events_[count++] = iter->first;
    }
    int nready = WSAWaitForMultipleEvents((DWORD)count, events_, FALSE, timeout, FALSE);
    if (nready == WSA_WAIT_FAILED)
    {
        LOG(ERROR) << "WSAWaitForMultipleEvents: " << LAST_ERROR_MSG;
        return 0;
    }
    else if (nready == WSA_WAIT_TIMEOUT)
    {
        UpdateTimer();
    }
    else if (nready == WSA_WAIT_IO_COMPLETION)
    {
        // Alertable I/O
    }
    else
    {
        int index = nready - WSA_WAIT_EVENT_0;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            LOG(ERROR) << "WSA wait events index out of range: " << index;
            return -1;
        }
        WSAEVENT hEvent = events_[index];
        OverlapFd* data = GetOverlapFdByEvent(hEvent);
        if (data == nullptr)
        {
            LOG(ERROR) << "GetHandleData: not found";
            return -1;
        }

        DWORD dwBytes = 0;
        DWORD dwFlags = 0;
        data->err = 0;
        if (!WSAGetOverlappedResult(data->fd, &data->overlap, &dwBytes, 0, &dwFlags))
        {
            data->err = WSAGetLastError();
        }
        DispatchEvent(data);
        return 1;
    }
    return 0;
}

void OverlappedIOService::DispatchEvent(OverlapFd* ev)
{
    switch(ev->op)
    {
    case OpConnect:
        {
            auto cb = boost::any_cast<ConnectCallback>(ev->ctx);
            if (cb)
            {
                cb(ev);
            }
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
