// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncEventPoller.h"
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Mask.h"
#include <vector>


AsyncEventPoller::AsyncEventPoller()
{
    memset(events_, 0, sizeof(events_));
}

AsyncEventPoller::~AsyncEventPoller()
{
    CleanUp();
}

void AsyncEventPoller::CleanUp()
{
    for (auto iter = eventFds_.begin(); iter != eventFds_.end();++iter)
    {
        WSAEVENT hEvent = iter->first;
        FdEntry* entry = iter->second;
        WSAEventSelect(entry->fd, NULL, 0);
        WSACloseEvent(hEvent);
        closesocket(entry->fd);
        delete entry;
    }
    eventFds_.clear();
    fdEvents_.clear(); 
}

int AsyncEventPoller::AddFd(SOCKET fd, IPollEvent* event)
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
    
    // automatically sets socket s to nonblocking mode
    long lEvents = FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE;
    int r = WSAEventSelect(fd, hEvent, lEvents);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << "WSAEventSelect: " << LAST_ERROR_MSG;
        WSACloseEvent(hEvent);
        return -1;
    }
    FdEntry* entry = new FdEntry;
    entry->fd = fd;
    entry->hEvent = hEvent;
    entry->sink = event;
    entry->mask = 0;
    eventFds_[hEvent] = entry;
    fdEvents_[fd] = entry;
    return 0;
}

void AsyncEventPoller::RemoveFd(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter == fdEvents_.end())
    {
        return; // not found
    }

    WSAEVENT hEvent = iter->second;

    //clear the event record associated with socket
    WSAEventSelect(fd, NULL, 0); 
    WSACloseEvent(hEvent);
    eventFds_.erase(hEvent);
    fdEvents_.erase(fd);
}

void AsyncEventPoller::SetPollIn(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter != fdEvents_.end())
    {
        FdEntry* entry = iter->second;
        entry->mask |= MASK_READABLE;
    }
}

void AsyncEventPoller::ResetPollIn(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter != fdEvents_.end())
    {
        FdEntry* entry = iter->second;
        entry->mask &= ~MASK_READABLE;
    }
}

void AsyncEventPoller::SetPollOut(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter != fdEvents_.end())
    {
        FdEntry* entry = iter->second;
        entry->mask |= MASK_WRITABLE;
    }
}

void AsyncEventPoller::ResetPollOut(SOCKET fd)
{
    auto iter = fdEvents_.find(fd);
    if (iter != fdEvents_.end())
    {
        FdEntry* entry = iter->second;
        entry->mask &= ~MASK_WRITABLE;
    }
}

int AsyncEventPoller::Poll(int timeout)
{
    if (fdEvents_.empty())
    {
        UpdateTimer();
        Sleep(timeout);
        return 0;
    }
    int count = 0;
    for (auto iter = eventFds_.begin(); iter != eventFds_.end(); ++iter)
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
            return 0;
        }
        WSAEVENT hEvent = events_[index];
        FdEntry* entry = eventFds_[hEvent];
        WSANETWORKEVENTS events = {};

        // This will reset the event object and adjust the status of 
        // active FD events on the socket in an atomic fashion.
        int r = WSAEnumNetworkEvents(entry->fd, hEvent, &events);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "WSAEnumNetworkEvents: " << LAST_ERROR_MSG;
            return 0;
        }
        HandleEvents(entry, &events);
        return 1;
    }
    return 0;
}

void AsyncEventPoller::HandleEvents(FdEntry* entry, WSANETWORKEVENTS* events)
{
    const int* errlist = events->iErrorCode;
    long event = events->lNetworkEvents;
    if (event & FD_READ)
    {
        int err = errlist[FD_READ_BIT];
        last_err_ = err;
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnReadable();
        }
    }
    if (event & FD_CLOSE)
    {
        int err = errlist[FD_CLOSE_BIT];
        last_err_ = err;
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnReadable();
        }
    }
    if (event & FD_ACCEPT)
    {
        int err = errlist[FD_ACCEPT_BIT];
        last_err_ = err;
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnReadable();
        }
    }
    if (event & FD_WRITE)
    {
        int err = errlist[FD_WRITE_BIT];
        last_err_ = err;
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnWritable();
        }
    }
    if (event & FD_CONNECT)
    {
        int err = errlist[FD_CONNECT_BIT];
        last_err_ = err;
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnWritable();
        }
    }
}
