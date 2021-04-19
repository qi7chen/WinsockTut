// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>
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
        SOCKET      fd;
        WSAEVENT    hEvent;
        LONG        lEvents;
        int         mask;
        IPollEvent* sink;
    };

    void CleanUp();
    void HandleEvents(FdEntry* entry, WSANETWORKEVENTS* events);
    FdEntry* FindEntry(SOCKET fd);
    FdEntry* FindEntryByEvent(WSAEVENT hEvent);
    void RemoveRetired();

private:
    std::vector<WSAEVENT>   events_;
    std::vector<FdEntry>    fds_;
    bool    has_retired_;
};
