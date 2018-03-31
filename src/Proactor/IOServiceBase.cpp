// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "IOServiceBase.h"


IOServiceBase::IOServiceBase()
{
    counter_ = 2018;
}

IOServiceBase::~IOServiceBase()
{

}

int IOServiceBase::AddTimer(int millsec, TimerCallback cb)
{
    TimerEntry entry = {};
    entry.id = counter_++;
    entry.expire = GetTickCount64() + millsec;
    entry.cb = cb;
    tree_.insert(entry);
    return entry.id;
}

// Complexity of this operation is O(n). We assume it is rarely used.
void IOServiceBase::CancelTimer(int id)
{
    for (auto iter = tree_.begin(); iter != tree_.end(); ++iter)
    {
        if (iter->id == id)
        {
            tree_.erase(iter);
            break;
        }
    }
}

void IOServiceBase::UpdateTimer()
{
    int64_t now = GetTickCount64();
    while (!tree_.empty())
    {
        auto iter = tree_.begin();
        if (now < iter->expire)
        {
            break;
        }
        tree_.erase(iter);
        if (iter->cb)
        {
            iter->cb();
        }
    }
}

