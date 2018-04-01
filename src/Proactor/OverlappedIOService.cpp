// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "OverlappedIOService.h"
#include <WS2tcpip.h>
#include "Common/Any.h"
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "WsaExt.h"


struct OverlappedIOService::PerHandleData
{
    WSAOVERLAPPED   overlap;
    WSABUF          wsbuf;
    SOCKET          fd;
    boost::any      callback;
};

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
        FreeHandleData(iter->second);
    }
    sock_handles_.clear();
    event_socks_.clear();
}

OverlappedIOService::PerHandleData* OverlappedIOService::AllocHandleData(SOCKET fd)
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
    PerHandleData* data = new PerHandleData;
    memset(data, 0, sizeof(*data));
    data->fd = fd;
    data->overlap.hEvent = hEvent;
    sock_handles_[fd] = data;
    event_socks_[hEvent] = fd;
    return data;
}

void OverlappedIOService::FreeHandleData(PerHandleData* data)
{
    sock_handles_.erase(data->fd);
    event_socks_.erase(data->overlap.hEvent);
    CloseHandle(data->overlap.hEvent);
    closesocket(data->fd);
    delete data;
}

OverlappedIOService::PerHandleData* OverlappedIOService::GetHandleData(WSAEVENT hEvent)
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

int OverlappedIOService::AsyncConnect(const std::string& addr, const std::string& port, ConnectCallback cb)
{
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;        // both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(addr.c_str(), port.c_str(), &hints, &aiList);
    if (err != 0)
    {
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", addr.c_str(), port.c_str(), 
            gai_strerror(err));
        return err;
    }
    SOCKET fd = INVALID_SOCKET;
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        // fd has the overlapped attribute as a default.
        fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
            continue; // try next addrinfo
        }

        // ConnectEx need previously bound socket.
        int r = bind(fd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "bind: " << LAST_ERROR_MSG;
            closesocket(fd);
            continue;
        }

        PerHandleData* data = AllocHandleData(fd);
        CHECK(data != nullptr) << "AllocHandleData";

        r = WsaExt::ConnectEx(fd, (const struct sockaddr*)pinfo->ai_addr, (int)pinfo->ai_addrlen, 
            nullptr, 0, nullptr, (OVERLAPPED*)data);
        if (r != TRUE)
        {
            if (GetLastError() != WSA_IO_PENDING)
            {
                LOG(ERROR) << "ConenctEx: " << LAST_ERROR_MSG;
                FreeHandleData(data);
                continue;
            }
        }
        else
        {
            // enable previously set properties or options
            r = setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
            if (r != 0)
            {
                FreeHandleData(data);
                delete data;
            }
        }
        data->callback = cb;
        break; // succeed
    }
    freeaddrinfo(aiList);
    return 0;
}

int OverlappedIOService::AsyncListen(SOCKET fd, const std::string& addr, const std::string& port, AcceptCallback cb)
{
    return 0;
}

int OverlappedIOService::AsyncRead(SOCKET fd, void* buf, int size, ReadCallback cb)
{
    return 0;
}

int OverlappedIOService::AsyncWrite(SOCKET fd, void* buf, int size, WriteCallback cb)
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
        PerHandleData* data = GetHandleData(hEvent);
        if (data == nullptr)
        {
            LOG(ERROR) << "GetHandleData: not found";
            return -1;
        }

        DWORD dwBytes = 0;
        DWORD dwFlags = 0;
        if (!WSAGetOverlappedResult(data->fd, &data->overlap, &dwBytes, 0, &dwFlags))
        {
            LOG(ERROR) << "WSAGetOverlappedResult: " << LAST_ERROR_MSG;
            return -1;
        }
        //data->callback();
        return 1;
    }
    return 0;
}