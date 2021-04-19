// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.


#include "SelectPoller.h"
#include <assert.h>
#include "Common/Error.h"
#include "Common/Logging.h"
#include <algorithm>


SelectPoller::SelectPoller()
{
    fds_.reserve(64);
  
    memset(&readfds_, 0, sizeof(fd_set));
    memset(&writefds_, 0, sizeof(fd_set));
    memset(&exceptfds_, 0, sizeof(fd_set));
}

SelectPoller::~SelectPoller()
{
}

int SelectPoller::AddFd(SOCKET fd, IPollEvent* event)
{
    assert(event != nullptr);
    if (fds_.size() >= FD_SETSIZE)
    {
        return -1;
    }
	FdEntry entry;
	entry.fd = fd;
	entry.sink = event;
    fds_[fd] = entry;

    // start polling on errors.
    FD_SET(fd, &exceptfds_);

    return 0;
}

void SelectPoller::RemoveFd(SOCKET fd)
{
	fds_[fd].fd = INVALID_SOCKET;
	fds_[fd].sink = nullptr;

    // stop polling on the descriptor
    FD_CLR(fd, &readfds_);
    FD_CLR(fd, &writefds_);
    FD_CLR(fd, &exceptfds_);
}

void SelectPoller::SetPollIn(SOCKET fd)
{
    FD_SET(fd, &readfds_);
}

void SelectPoller::ResetPollIn(SOCKET fd)
{
    FD_CLR(fd, &readfds_);
}

void SelectPoller::SetPollOut(SOCKET fd)
{
    FD_SET(fd, &writefds_);
}

void SelectPoller::ResetPollOut(SOCKET fd)
{
    FD_CLR(fd, &writefds_);
}

// timeout in milliseconds
int SelectPoller::Poll(int timeout)
{
	int r = 0;
	fd_set rdset, wrset, exptset;

	// select() returns WSAEINVAL if all 3 fd_sets empty
	if (!fds_.empty())
	{		
		memcpy(&rdset, &readfds_, sizeof(fd_set));
		memcpy(&wrset, &writefds_, sizeof(fd_set));
		memcpy(&exptset, &exceptfds_, sizeof(fd_set));
		timeval tvp = {};
		tvp.tv_usec = timeout * 1000;
		r = select(0, &rdset, &wrset, &exptset, &tvp);
		if (r == SOCKET_ERROR)
		{
			LOG(ERROR) << GetLastError() << ": " << LAST_ERROR_MSG;
			return 0;
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
    
    int count = 0;
	auto iter = fds_.begin();
	while (iter != fds_.end())
	{
        SOCKET fd = iter->first;
		FdEntry entry = iter->second;
		if (entry.fd == INVALID_SOCKET || entry.sink == nullptr)
		{
			iter = fds_.erase(iter);
			continue;
		}
        if (FD_ISSET(fd, &exptset))
        {
			entry.sink->OnReadable();
            count++;
        }

        if (FD_ISSET(fd, &wrset))
        {
			entry.sink->OnWritable();
            count++;
        }

        if (FD_ISSET(fd, &rdset))
        {
			entry.sink->OnReadable();
            count++;
        }
	}

	// execute time out callbacks
	UpdateTimer(); 

    return count;
}
