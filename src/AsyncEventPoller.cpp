// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncEventPoller.h"
#include <algorithm>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Mask.h"
#include <vector>


AsyncEventPoller::AsyncEventPoller()
    : has_retired_(false)
{
    events_.reserve(WSA_MAXIMUM_WAIT_EVENTS);
    fds_.reserve(WSA_MAXIMUM_WAIT_EVENTS);
}

AsyncEventPoller::~AsyncEventPoller()
{
    CleanUp();
}

void AsyncEventPoller::CleanUp()
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        FdEntry* entry = &fds_[i];
        WSAEventSelect(entry->fd, NULL, 0);
        WSACloseEvent(entry->hEvent);
        closesocket(entry->fd);
        entry->fd = INVALID_SOCKET;
        entry->hEvent = NULL;
    }
    fds_.clear();
    events_.clear();
}

int AsyncEventPoller::AddFd(SOCKET fd, IPollEvent* event)
{
    if (fds_.size() >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        return -1;
    }
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        LOG(ERROR) << "WSACreateEvent: " << LAST_ERROR_MSG;
        return -1;
    }
    FdEntry entry = {};
    entry.fd = fd;
    entry.hEvent = hEvent;
    entry.sink = event;
    fds_.push_back(entry);
    
    return 0;
}

void AsyncEventPoller::RemoveFd(SOCKET fd)
{
    FdEntry* entry = FindEntry(fd);
    if (entry != NULL)
    {
        //clear the event record associated with socket
        WSACloseEvent(entry->hEvent);
        entry->hEvent = WSA_INVALID_EVENT;
        entry->fd = INVALID_SOCKET; // mark retired
    }
    has_retired_ = true;
    WSAEventSelect(fd, NULL, 0);
}

void AsyncEventPoller::SetPollIn(SOCKET fd)
{
    FdEntry* entry = FindEntry(fd);
    if (entry != NULL)
    {
        long lEvent = FD_READ | FD_ACCEPT | FD_CLOSE;
        entry->lEvents |= lEvent;
        entry->mask |= MASK_READABLE;
        int r = WSAEventSelect(fd, entry->hEvent, entry->lEvents);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "SetPollIn: WSAEventSelect, " << LAST_ERROR_MSG;
            return;
        }
    }
}

void AsyncEventPoller::ResetPollIn(SOCKET fd)
{
    FdEntry* entry = FindEntry(fd);
    if (entry != NULL)
    {
        long lEvent = FD_READ | FD_ACCEPT | FD_CLOSE;
        entry->lEvents &= ~lEvent;
        entry->mask &= ~MASK_READABLE;
        int r = WSAEventSelect(fd, entry->hEvent, entry->lEvents);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "ResetPollIn: WSAEventSelect, " << LAST_ERROR_MSG;
            return;
        }
    }
}

void AsyncEventPoller::SetPollOut(SOCKET fd)
{
    FdEntry* entry = FindEntry(fd);
    if (entry != NULL)
    {
        long lEvent = FD_WRITE | FD_CONNECT | FD_CLOSE;
        entry->lEvents |= lEvent;
        entry->mask |= MASK_WRITABLE;
        int r = WSAEventSelect(fd, entry->hEvent, entry->lEvents);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "SetPollOut: WSAEventSelect, " << LAST_ERROR_MSG;
            return;
        }
    }
}

void AsyncEventPoller::ResetPollOut(SOCKET fd)
{
    FdEntry* entry = FindEntry(fd);
    if (entry != NULL)
    {
        long lEvent = FD_WRITE | FD_CONNECT | FD_CLOSE;
        entry->lEvents &= ~lEvent;
        entry->mask &= ~MASK_WRITABLE;
        int r = WSAEventSelect(fd, entry->hEvent, entry->lEvents);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "ResetPollOut: WSAEventSelect, " << LAST_ERROR_MSG;
            return;
        }
    }
}

int AsyncEventPoller::Poll(int timeout)
{
    int nready = 0;
    events_.clear();
    for (size_t i = 0; i < fds_.size(); i++)
    {
        FdEntry* entry = &fds_[i];
        if (entry->fd != INVALID_SOCKET && entry->hEvent != WSA_INVALID_EVENT)
        {
            events_.push_back(entry->hEvent);
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
            return 0;
        }
        WSAEVENT hEvent = events_[index];
        FdEntry* entry = FindEntryByEvent(hEvent);
        if (entry == NULL)
        {
            LOG(ERROR) << "Poll: entry not found";
            return 0;
        }
        WSANETWORKEVENTS events = {};

        // This will reset the event object and adjust the status of 
        // active FD events on the socket in an atomic fashion.
        int r = WSAEnumNetworkEvents(entry->fd, hEvent, &events);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << "Poll: WSAEnumNetworkEvents, " << LAST_ERROR_MSG;
            return 0;
        }
        HandleEvents(entry, &events);
        return 1;
    }
    RemoveRetired();
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

AsyncEventPoller::FdEntry* AsyncEventPoller::FindEntry(SOCKET fd)
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        if (fds_[i].fd == fd)
        {
            return &fds_[i];
        }
    }
    return NULL;
}

AsyncEventPoller::FdEntry* AsyncEventPoller::FindEntryByEvent(WSAEVENT hEvent)
{
    for (size_t i = 0; i < fds_.size(); i++)
    {
        if (fds_[i].hEvent == hEvent)
        {
            return &fds_[i];
        }
    }
    return NULL;
}

void AsyncEventPoller::RemoveRetired()
{
    if (has_retired_)
    {
        auto iter = std::remove_if(fds_.begin(), fds_.end(), [](const FdEntry& entry)
        {
            return entry.fd == INVALID_SOCKET;
        });
        fds_.erase(iter, fds_.end());
        has_retired_ = false;
    }
}
