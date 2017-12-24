// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "select.h"
#include <assert.h>
#include "Common/Logging.h"
#include "Common/Util.h"
#include "EventLoop.h"

SelectPoller::SelectPoller()
{
    memset(&readfds_, 0, sizeof(fd_set));
    memset(&writefds_, 0, sizeof(fd_set));
    memset(&exceptfds_, 0, sizeof(fd_set));
}

SelectPoller::~SelectPoller()
{
}

void SelectPoller::AddFd(SOCKET fd, int mask)
{
    if (mask & EV_READABLE)
    {
        assert(readfds_.fd_count < FD_SETSIZE);
        FD_SET(fd, &readfds_);
        FD_SET(fd, &exceptfds_);
    }
    if (mask & EV_WRITABLE) 
    {
        assert(writefds_.fd_count < FD_SETSIZE);
        FD_SET(fd, &writefds_);
        FD_SET(fd, &exceptfds_);
    }
}

void SelectPoller::DelFd(SOCKET fd, int mask)
{
    if (mask & EV_READABLE)
    {
        FD_CLR(fd, &readfds_);
        FD_CLR(fd, &exceptfds_);
    }
    if (mask & EV_WRITABLE) 
    {
        FD_CLR(fd, &writefds_);
        FD_CLR(fd, &exceptfds_);
    }
}

// timeout in milliseconds
int SelectPoller::Poll(EventLoop* loop, int timeout)
{
    timeval tvp = {};
	tvp.tv_usec = timeout * 1000;

    fd_set rdset, wrset, exptset;
	memcpy(&rdset, &readfds_, sizeof(fd_set));
	memcpy(&wrset, &writefds_, sizeof(fd_set));
    memcpy(&exptset, &exceptfds_, sizeof(fd_set));

	int r = select(0, &rdset, &wrset, &exptset, &tvp);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << LAST_ERROR_MSG;
        return 0;
    } else if (r == 0) { // timed-out
        return 0;
    }
    int count = 0;
    const std::unordered_map<SOCKET, EventEntry>& dict = loop->GetEventDict();
    for (auto iter = dict.begin(); iter != dict.end(); ++iter)
    {
        int mask = 0;
        SOCKET fd = iter->first;
        const EventEntry* entry = &iter->second;
        if (entry->mask == EV_NONE)
            continue;
        if ((entry->mask & EV_READABLE) && FD_ISSET(fd, &rdset))
        {
            mask |= EV_READABLE;
        }
        if ((entry->mask & EV_WRITABLE) && FD_ISSET(fd, &wrset))
        {
            mask |= EV_WRITABLE;
        }
        if (FD_ISSET(fd, &exptset))
        {
            mask |= EV_READABLE;
            mask |= EV_EXCEPT;
        }
        if (mask != 0)
        {
            loop->AddFiredEvent(fd, mask);
            count++;
        }
    }
    return count;
}
