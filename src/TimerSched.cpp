// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "TimerSched.h"
#include <stdint.h>
#include <Windows.h>    // GetTickCount64

struct TimerSched::TimerEntry
{
    int         index;  // heap index of this item
    int         id;     // unique timer id
    int64_t     expire; // expire time
    ITimerEvent* sink;  // event sink

    TimerEntry()
    {
        index = 0;
        id = 0;
        expire = 0;
        sink = nullptr;
    }
};

TimerSched::TimerSched()
    : next_id_(100) // time flies
{
    ref_.rehash(64);
}

TimerSched::~TimerSched()
{
    clear();
}

void TimerSched::clear()
{
    for (size_t i = 0; i < heap_.size(); i++)
    {
        delete heap_[i];
    }
    heap_.clear();
}

#define HEAP_ITEM_LESS(i, j) (heap_[(i)]->expire < heap_[(j)]->expire)

bool TimerSched::siftdown(int x, int n)
{
    int i = x;
    for (;;)
    {
        int j1 = 2 * i + 1;
        if ((j1 >= n) || (j1 < 0)) // j1 < 0 after int overflow
        {
            break;
        }
        int j = j1; // left child
        int j2 = j1 + 1;
        if (j2 < n && !HEAP_ITEM_LESS(j1, j2))
        {
            j = j2; // right child
        }
        if (!HEAP_ITEM_LESS(j, i))
        {
            break;
        }
        std::swap(heap_[i], heap_[j]);
        heap_[i]->index = i;
        heap_[j]->index = j;
        i = j;
    }
    return i > x;
}

void TimerSched::siftup(int j)
{
    for (;;)
    {
        int i = (j - 1) / 2; // parent node
        if (i == j || !HEAP_ITEM_LESS(j, i))
        {
            break;
        }
        std::swap(heap_[i], heap_[j]);
        heap_[i]->index = i;
        heap_[j]->index = j;
        j = i;
    }
}

#undef HEAP_ITEM_LESS

int TimerSched::AddTimer(int millsec, ITimerEvent* event)
{
    TimerEntry* entry = new TimerEntry;
    entry->id = next_id_++;
    entry->expire = GetTickCount64() + millsec;
    entry->sink = event;
    entry->index = (int)heap_.size();
    ref_[entry->id] = entry;
    heap_.push_back(entry);
    siftup((int)heap_.size() - 1);
    return entry->id;
}

void TimerSched::UpdateTimer()
{
    int64_t now = GetTickCount64();
    int last_id = next_id_;
    while (!heap_.empty())
    {
        TimerEntry* entry = heap_[0];
        if (now < entry->expire)
        {
            break;
        }
        // we don't process timers created by time events in this iteration
        if (entry->id > last_id) {
            continue;
        }
        int n = (int)heap_.size() - 1;
        std::swap(heap_[0], heap_[n]);
        heap_[0]->index = 0;
        siftdown(0, n);
        heap_.pop_back();
        ref_.erase(entry->id);
        if (entry->sink)
        {
            entry->sink->OnTimeout();
        }
        delete entry;
    }
}

void TimerSched::CancelTimer(int id)
{
    auto iter = ref_.find(id);
    if (iter == ref_.end())
    {
        return;
    }
    TimerEntry* entry = iter->second;
    if (entry == nullptr)
    {
        return;
    }
    int n = (int)heap_.size() - 1;
    int i = entry->index;
    if (i != n)
    {
        std::swap(heap_[i], heap_[n]);
        heap_[i]->index = i;
        if (!siftdown(i, n))
        {
            siftup(i);
        }
    }
    heap_.pop_back();
    delete entry;
}