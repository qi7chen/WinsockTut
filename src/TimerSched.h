// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <vector>
#include <unordered_map>
#include "PollEvent.h"

class TimerSched
{
public:
    TimerSched();
    virtual ~TimerSched();
    
    int     AddTimer(int millsec, ITimerEvent* event);
    void    CancelTimer(int id);
    void    UpdateTimer();

private:
    void clear();
    bool siftdown(int x, int n);
    void siftup(int j);

    struct TimerEntry;
private:
    int     next_id_;                           // next timer id
    std::vector<TimerEntry*>  heap_;            // min-heap timer
    std::unordered_map<int, TimerEntry*> ref_;  // reference
};
