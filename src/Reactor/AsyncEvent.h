// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include <unordered_map>
#include "Common/Define.h"


class AsyncEventPoller : public IOPoller
{
public:
    AsyncEventPoller();
    ~AsyncEventPoller();

    int AddFd(SOCKET fd, int mask);
    void DelFd(SOCKET fd, int mask);
    int Poll(EventLoop* loop, int timeout);

private:
    void HandleEvents(EventLoop* loop, SOCKET fd, WSANETWORKEVENTS* events);

private:
    std::vector<WSAEVENT> events_;
    std::unordered_map<SOCKET, WSAEVENT> fdEvents_;
    std::unordered_map<WSAEVENT, SOCKET> eventFds_;
};
