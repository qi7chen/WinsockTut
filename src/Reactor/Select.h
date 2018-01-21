// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.
 
#pragma once

#include "Common/Define.h"

class SelectPoller : public IOPoller
{
public:
    SelectPoller();
    ~SelectPoller();

    int AddFd(SOCKET fd);
    void DeleteFd(SOCKET fd);
    int Poll(EventLoop* loop, int timeout);

private:
    fd_set		readfds_;
	fd_set		writefds_;
    fd_set		exceptfds_;
};
