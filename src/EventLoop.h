// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "Common/Define.h"
#include <vector>
#include <unordered_map>

struct EventEntry
{
    int         mask;
    SOCKET      fd;
    EventProc   readProc;
    EventProc   writeProc;

    EventEntry()
    {
        mask = 0;
        fd = 0;
        readProc = NULL;
        writeProc = NULL;
    }
};

class EventLoop
{
public:
    explicit EventLoop(int type);
    ~EventLoop();

    //
    void CreateEvent(SOCKET fd, int mask, EventProc func);

    //
    void DeleteEvent(SOCKET fd, int mask);

    //
    EventEntry* GetEntry(SOCKET fd);

    const std::unordered_map<SOCKET, EventEntry>& GetEventDict() { return events_; }

    void AddFiredEvent(SOCKET fd, int mask);

    void Run();

    void RunOne();

private:
    void createMultiplexer(int type);

private:
    int timeout_;
    IMultiplexer* state_;
    std::unordered_map<SOCKET, EventEntry> events_;
    std::unordered_map<SOCKET, int> fired_;
};