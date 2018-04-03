// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include "PollerBase.h"

class AsyncEventPoller : public PollerBase
{
public:
    AsyncEventPoller();
    ~AsyncEventPoller();

    int AddFd(SOCKET fd, IPollEvent* event);
    void RemoveFd(SOCKET fd);

    void SetPollIn(SOCKET fd);
    void ResetPollIn(SOCKET fd);
    void SetPollOut(SOCKET fd);
    void ResetPollOut(SOCKET fd);

    int Poll(int timeout);

private:
    struct FdEntry
    {
        SOCKET fd;
        WSAEVENT hEvent;
        IPollEvent* sink;
        int mask;
    };

    void CleanUp();
    void HandleEvents(FdEntry* entry, WSANETWORKEVENTS* events);

private:
    WSAEVENT                        events_[WSA_MAXIMUM_WAIT_EVENTS];
    std::map<SOCKET, FdEntry*>      fdEvents_;
    std::map<WSAEVENT, FdEntry*>    eventFds_;
};
