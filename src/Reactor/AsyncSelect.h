// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>
#include "Common/Define.h"
#include <unordered_map>

// TO-DO: WSAAsyncSelect implementation still has bug
class AsyncSelectPoller : public IOPoller
{
public:
    AsyncSelectPoller();
    ~AsyncSelectPoller();

    int AddFd(SOCKET fd);
    void DeleteFd(SOCKET fd);
    int Poll(EventLoop* loop, int timeout);

private:
    void HandleEvent(EventLoop* loop, SOCKET fd, int event, int error);
    void CreateHidenWindow();

private:
    HWND    hwnd_;
};
