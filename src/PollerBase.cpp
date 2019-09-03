// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "PollerBase.h"
#include <stdint.h>
#include <Windows.h>
#include "SelectPoller.h"
#include "AsyncSelectPoller.h"
#include "AsyncEventPoller.h"

struct PollerBase::TimerEntry
{
    int index;
    int id;
    int64_t expire;
    IPollEvent* sink;

    TimerEntry()
    {
        index = 0;
        id = 0;
        expire = 0;
        sink = nullptr;
    }
};

PollerBase::PollerBase()
{
    last_err_ = 0;
    counter_ = 2018;
    heap_.reserve(64);
}

PollerBase::~PollerBase()
{
    clear();
}

void PollerBase::clear()
{
    for (size_t i = 0; i < heap_.size(); i++)
    {
        delete heap_[i];
    }
    heap_.clear();
}

#define HEAP_ITEM_LESS(i, j) (heap_[(i)]->expire < heap_[(j)]->expire)

bool PollerBase::siftdown(int x, int n)
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

void PollerBase::siftup(int j)
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

int PollerBase::AddTimer(int millsec, IPollEvent* event)
{
    TimerEntry* entry = new TimerEntry;
    entry->id = counter_++;
    entry->expire = GetTickCount64() + millsec;
    entry->sink = event;
    entry->index = (int)heap_.size();
    heap_.push_back(entry);
    siftup((int)heap_.size() - 1);
    return entry->id;
}

void PollerBase::UpdateTimer()
{
    int64_t now = GetTickCount64();
    while (!heap_.empty())
    {
        TimerEntry* entry = heap_[0];
        if (now < entry->expire)
        {
            break;
        }
        int n = (int)heap_.size() - 1;
        std::swap(heap_[0], heap_[n]);
        heap_[0]->index = 0;
        siftdown(0, n);
        heap_.pop_back();
        if (entry->sink)
        {
            entry->sink->OnTimeout();
        }
        delete entry;
    }
}

void PollerBase::CancelTimer(int id)
{
    TimerEntry* entry = nullptr;
    for (size_t i = 0; i < heap_.size(); i++)
    {
        if (heap_[i]->id == id)
        {
            entry = heap_[i];
            break;
        }
    }
    if (entry == nullptr)
    {
        return ;
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

PollerBase* CreatePoller(PollerType type)
{
    switch(type)
    {
    case PollerSelect:
        return new SelectPoller;

    case PollerAsyncSelect:
        return new AsyncSelectPoller;

    case PollerAsyncEvent:
        return new AsyncEventPoller;

    default:
        return nullptr;
    }
}
