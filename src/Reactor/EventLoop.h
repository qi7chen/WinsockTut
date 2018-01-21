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
    SOCKET      fd;
    EventProc   callback;
};

struct FiredEvent
{
    SOCKET fd;
    int event;
    int err;
};

class EventLoop
{
public:
    explicit EventLoop(IOMode type);
    ~EventLoop();

    //
    int AddEvent(SOCKET fd, EventProc func);

    //
    void DelEvent(SOCKET fd);

    //
    EventEntry* GetEntry(SOCKET fd);

    const std::unordered_map<SOCKET, EventEntry*>& GetEventDict() 
    { 
        return events_; 
    }

    void AddFiredEvent(SOCKET fd, int mask, int err);

    void Run();

    void RunOne();

private:
    void createIOPoller(IOMode type);

private:
    int         timeout_;
    IOPoller*   poller_;

    std::unordered_map<SOCKET, EventEntry*>  events_;
    std::vector<FiredEvent>  fired_;
};
