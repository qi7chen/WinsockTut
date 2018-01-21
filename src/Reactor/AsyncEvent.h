// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include <unordered_map>
#include "Common/Define.h"

// TO-DO: WSAEventSelect implementation still has bug
class AsyncEventPoller : public IOPoller
{
public:
    AsyncEventPoller();
    ~AsyncEventPoller();

    int AddFd(SOCKET fd);
    void DeleteFd(SOCKET fd);
    int Poll(EventLoop* loop, int timeout);

private:
    void HandleEvents(EventLoop* loop, SOCKET fd, WSANETWORKEVENTS* events);
    void CleanUp();

private:
    std::unordered_map<SOCKET, WSAEVENT> fdEvents_;
    std::unordered_map<WSAEVENT, SOCKET> eventFds_;
};
