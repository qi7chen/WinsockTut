// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.
 
#pragma once

#include "PollerBase.h"
#include <vector>

class SelectPoller : public PollerBase
{
public:
    SelectPoller();
    ~SelectPoller();

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
        IPollEvent* sink;
    };

private:
    std::vector<FdEntry>    fds_;
    bool has_retired_; 

    fd_set      readfds_;
    fd_set      writefds_;
    fd_set      exceptfds_;
};
