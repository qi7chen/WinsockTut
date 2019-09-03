// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "TimerSched.h"
#include <WinSock2.h>
#include <vector>
#include <map>

enum PollerType
{
    PollerSelect = 1,
    PollerAsyncSelect = 2,
    PollerAsyncEvent = 3,
};

class PollerBase : public TimerSched
{
public:
    PollerBase();
    virtual ~PollerBase();

    virtual int AddFd(SOCKET fd, IPollEvent* event) = 0;
    virtual void RemoveFd(SOCKET fd) = 0;

    virtual void SetPollIn(SOCKET fd) = 0;
    virtual void ResetPollIn(SOCKET fd) = 0;
    virtual void SetPollOut(SOCKET fd) = 0;
    virtual void ResetPollOut(SOCKET fd) = 0;
    virtual int Poll(int timeout) = 0;

    int LastError() { return last_err_;  }


protected:
    int last_err_;
};

PollerBase* CreatePoller(PollerType type);
