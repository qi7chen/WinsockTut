// Copyright (C) 2012-2018 . All rights reserved.
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
    has_retired_ = false;
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
    FdEntry entry = {fd, event};
    fds_.push_back(entry);

    // start polling on errors.
    FD_SET(fd, &exceptfds_);

    return 0;
}

void SelectPoller::RemoveFd(SOCKET fd)
{
    // mark the descriptor as retired.
    for (size_t i = 0; i < fds_.size(); i++)
    {
        if (fds_[i].fd == fd)
        {
            fds_[i].fd = INVALID_SOCKET;
			fds_[i].sink = NULL;
            break;
        }
    }
    has_retired_ = true;

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
	if (!fds_.empty()) // WSAEINVAL if all 3 fd_sets empty
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
		Sleep(timeout / 2);
	}
    
    int count = 0;
	for (size_t i = 0; r > 0 && i < fds_.size(); i++)
	{
		SOCKET fd = fds_[i].fd;
		if (fd == INVALID_SOCKET)
			continue;

		if (FD_ISSET(fd, &exptset))
		{
			fds_[i].sink->OnReadable();
			count++;
		}
		if (fd == INVALID_SOCKET)
			continue;

		if (FD_ISSET(fd, &wrset))
		{
			fds_[i].sink->OnWritable();
			count++;
		}
		if (fd == INVALID_SOCKET)
			continue;

		if (FD_ISSET(fd, &rdset))
		{
			fds_[i].sink->OnReadable();
			count++;
		}
	}

	// execute time out callbacks
	UpdateTimer(); 

	RemoveRetired();

    return count;
}

void SelectPoller::RemoveRetired()
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