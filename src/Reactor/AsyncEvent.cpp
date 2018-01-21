// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncEvent.h"
#include "Common/Logging.h"
#include "Common/Util.h"
#include "EventLoop.h"

AsyncEventPoller::AsyncEventPoller()
{
    fdEvents_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
    eventFds_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
}

AsyncEventPoller::~AsyncEventPoller()
{
    CleanUp();
}

void AsyncEventPoller::CleanUp()
{
    for (auto iter = eventFds_.begin(); iter != eventFds_.end();++iter)
    {
        WSAEventSelect(iter->second, NULL, 0);
        WSACloseEvent(iter->first);
        closesocket(iter->second);
    }
    eventFds_.clear();
    fdEvents_.clear();
}

int AsyncEventPoller::AddFd(SOCKET fd)
{
    if (fdEvents_.size() >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        return -1;
    }

    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        LOG(ERROR) << "WSACreateEvent: " << LAST_ERROR_MSG;
        return -1;
    }
    
    // associate event handle to socket
    long lEvents = FD_READ | FD_WRITE | FD_ACCEPT;
    int r = WSAEventSelect(fd, hEvent, lEvents);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << "WSAEventSelect: " << LAST_ERROR_MSG;
        WSACloseEvent(hEvent);
        return -1;
    }
    eventFds_[hEvent] = fd;
    fdEvents_[fd] = hEvent;
    return 0;
}

void AsyncEventPoller::DeleteFd(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter == fdEvents_.end())
    {
        return;
    }
    WSAEVENT hEvent = iter->second;
    WSAEventSelect(fd, NULL, 0);
    WSACloseEvent(hEvent);
    eventFds_.erase(hEvent);
    fdEvents_.erase(fd);
}

void AsyncEventPoller::HandleEvents(EventLoop* loop, SOCKET fd, WSANETWORKEVENTS* events)
{
    const int* errlist = events->iErrorCode;
    long event = events->lNetworkEvents;
    if (event & FD_READ)
    {
        loop->AddFiredEvent(fd, EV_READABLE, errlist[FD_READ_BIT]);
    }
    if (event & FD_ACCEPT)
    {
        loop->AddFiredEvent(fd, EV_READABLE, errlist[FD_ACCEPT_BIT]);
    }
    if (event & FD_WRITE)
    {
        loop->AddFiredEvent(fd, EV_WRITABLE, errlist[FD_WRITE_BIT]);
    }
}

int AsyncEventPoller::Poll(EventLoop* loop, int timeout)
{
    if (fdEvents_.empty())
    {
        return 0;
    }
    std::vector<WSAEVENT> events;
    events.reserve(WSA_MAXIMUM_WAIT_EVENTS);
    for (auto iter = fdEvents_.begin(); iter != fdEvents_.end(); ++iter)
    {
        events.push_back(iter->second);
    }
    int nready = WSAWaitForMultipleEvents(events.size(), &events[0], FALSE, timeout, false);
    if (nready == WSA_WAIT_FAILED)
    {
        LOG(ERROR) << "WSAWaitForMultipleEvents: " << LAST_ERROR_MSG;
        return 0;
    }
    else if (nready == WSA_WAIT_TIMEOUT)
    {
        // handle timers here
    }
    else if (nready == WSA_WAIT_IO_COMPLETION)
    {
        // Alertable I/O
    }
    else 
    {
        int index = WSA_WAIT_EVENT_0 + nready;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            LOG(ERROR) << "WSA wait events index out of range: " << index;
            return 0;
        }
        WSAEVENT hEvent = events[index];
        SOCKET fd = eventFds_[hEvent];
        WSANETWORKEVENTS events = {};
        if (!WSAResetEvent(hEvent))
        {
            LOG(ERROR) << "WSAResetEvent: " << LAST_ERROR_MSG;
            return 0;
        }
        int r = WSAEnumNetworkEvents(fd, hEvent, &events);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "WSAEnumNetworkEvents: " << LAST_ERROR_MSG;
            return 0;
        }
        HandleEvents(loop, fd, &events);
        return 1;
    }
}
