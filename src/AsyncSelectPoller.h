// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>
#include <map>
#include "PollerBase.h"

// This API may be altered or unavailable in subsequent Windows versions.
class AsyncSelectPoller : public PollerBase
{
public:
    AsyncSelectPoller();
    ~AsyncSelectPoller();

    int AddFd(SOCKET fd, IPollEvent* event);
    void RemoveFd(SOCKET fd);

    void SetPollIn(SOCKET fd);
    void ResetPollIn(SOCKET fd);
    void SetPollOut(SOCKET fd);
    void ResetPollOut(SOCKET fd);

    int Poll(int timeout);

private:
    void CreateHidenWindow();
    void WatchWritable();
    void HandleEvent(SOCKET fd, int ev, int ec);

    struct FdEntry
    {
        int         mask;
        IPollEvent* sink;
    };

private:
    HWND    hwnd_;
    std::map<SOCKET, FdEntry> fds_;
};
