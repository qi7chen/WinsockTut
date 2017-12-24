// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncEvent.h"
#include "Common/Logging.h"
#include "Common/Util.h"
#include "EventLoop.h"

AsyncEventPoller::AsyncEventPoller()
{
    events_.reserve(WSA_MAXIMUM_WAIT_EVENTS);
    fdEvents_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
    eventFds_.rehash(WSA_MAXIMUM_WAIT_EVENTS);
}

AsyncEventPoller::~AsyncEventPoller()
{
}

int AsyncEventPoller::AddFd(SOCKET fd, int mask)
{
    if (events_.size() >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG(ERROR) << "events too many";
        return -1;
    }
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        LOG(ERROR) << "WSACreateEvent: " << LAST_ERROR_MSG;
        return -1;
    }
    
    // associate event handle to socket
    long lEvents = 0;
    if (mask & EV_READABLE)
    {
        lEvents |= FD_READ;
        lEvents |= FD_ACCEPT;
    }
    if (mask & EV_WRITABLE)
    {
        lEvents |= FD_WRITE;
    }
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

void AsyncEventPoller::DelFd(SOCKET fd, int mask)
{
    WSAEventSelect(fd, NULL, 0);
    WSAEVENT hEvent = fdEvents_[fd];
    eventFds_.erase(hEvent);
    fdEvents_.erase(fd);
}

void AsyncEventPoller::HandleEvents(EventLoop* loop, SOCKET fd, WSANETWORKEVENTS* events)
{
    const int* errlist = events->iErrorCode;
    if (events->lNetworkEvents & FD_READ)
    {
        loop->AddFiredEvent(fd, EV_READABLE, errlist[FD_READ_BIT]);
    }
    if (events->lNetworkEvents & FD_WRITE)
    {
        loop->AddFiredEvent(fd, EV_WRITABLE, errlist[FD_WRITE_BIT]);
    }
    if (events->lNetworkEvents & FD_CLOSE)
    {
        loop->AddFiredEvent(fd, EV_READABLE, errlist[FD_CLOSE_BIT]);
    }
}

int AsyncEventPoller::Poll(EventLoop* loop, int timeout)
{
    events_.clear();
    for (auto iter = fdEvents_.begin(); iter != fdEvents_.end(); ++iter)
    {
        events_.push_back(iter->second);
    }
    if (events_.empty())
    {
        return 0;
    }
    int nready = WSAWaitForMultipleEvents(events_.size(), &events_[0], FALSE, timeout, false);
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
    }
    else 
    {
        int index = WSA_WAIT_EVENT_0 + nready;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            LOG(ERROR) << "WSA wait events index out of range: " << index;
            return 0;
        }
        WSAEVENT hEvent = events_[index];
        SOCKET fd = eventFds_[hEvent];
        WSANETWORKEVENTS events = {};
        int r = WSAEnumNetworkEvents(fd, hEvent, &events);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "WSAEnumNetworkEvents: " << LAST_ERROR_MSG;
            return 0;
        }
        HandleEvents(loop, fd, &events);
        return 1;
    }
    return 0;
}
