// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "select.h"
#include <assert.h>
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
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

int SelectPoller::AddFd(SOCKET fd)
{
    if(readfds_.fd_count == FD_SETSIZE)
    {
        return -1;
    }
    FD_SET(fd, &readfds_);
    FD_SET(fd, &exceptfds_);
    FD_SET(fd, &writefds_);
    return 0;
}

void SelectPoller::DeleteFd(SOCKET fd)
{
    FD_CLR(fd, &readfds_);
    FD_CLR(fd, &writefds_);
    FD_CLR(fd, &exceptfds_);
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
    } 
    else if (r == 0)  // timed-out
    {
        return 0;
    }
    int count = 0;
    const auto& dict = loop->GetEventDict();
    for (auto iter = dict.begin(); iter != dict.end(); ++iter)
    {
        int mask = 0;
        SOCKET fd = iter->first;
        const EventEntry* entry = iter->second;
        if (entry->fd == INVALID_SOCKET)
        {
            continue;
        }
        if (FD_ISSET(fd, &rdset))
        {
            mask |= EV_READABLE;
        }
        if (FD_ISSET(fd, &wrset))
        {
            mask |= EV_WRITABLE;
        }
        if (FD_ISSET(fd, &exptset))
        {
            mask |= EV_READABLE;
        }
        if (mask != 0)
        {
            loop->AddFiredEvent(fd, mask, 0);
            count++;
        }
    }
    return count;
}
