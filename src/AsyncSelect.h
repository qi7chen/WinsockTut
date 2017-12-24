// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>
#include "Common/Define.h"

class AsyncSelect : public IOPoller
{
public:
    AsyncSelect();
    ~AsyncSelect();

    int AddFd(SOCKET fd, int mask);
    void DelFd(SOCKET fd, int mask);
    int Poll(EventLoop* loop, int timeout);

private:
    void HandleEvent(EventLoop* loop, SOCKET fd, int event, int error);
    void CreateHidenWindow();

private:
    HWND    hwnd_;
};
