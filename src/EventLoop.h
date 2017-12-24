// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "Common/Define.h"
#include <vector>
#include <utility>
#include <unordered_map>

struct EventEntry
{
    int         mask;
    SOCKET      fd;
    EventProc   proc;
    
    EventEntry()
    {
        mask = 0;
        fd = INVALID_SOCKET;
    }
};

class EventLoop
{
public:
    explicit EventLoop(IOMode type);
    ~EventLoop();

    //
    void AddEvent(SOCKET fd, int mask, EventProc func);

    //
    void DelEvent(SOCKET fd, int mask);

    //
    EventEntry* GetEntry(SOCKET fd);

    const std::unordered_map<SOCKET, EventEntry>& GetEventDict() { return events_; }

    void AddFiredEvent(SOCKET fd, int mask, int ec);

    void Run();

    void RunOne();

private:
    void createIOPoller(IOMode type);

private:
    int         timeout_;
    IOPoller*   poller_;

    std::unordered_map<SOCKET, EventEntry>  events_;
    std::unordered_map<SOCKET, std::pair<int,int>>  fired_;
};
