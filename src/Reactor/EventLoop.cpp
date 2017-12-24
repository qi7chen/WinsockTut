// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EventLoop.h"
#include <assert.h>
#include "Common/Define.h"
#include "Common/Logging.h"
#include "Select.h"
#include "AsyncSelect.h"
#include "AsyncEvent.h"

EventLoop::EventLoop(IOMode type)
{
    timeout_ = 50;
    createIOPoller(type);
    events_.rehash(64);
    fired_.reserve(64);
}

EventLoop::~EventLoop()
{
    delete poller_;
    poller_ = NULL;
}

void EventLoop::createIOPoller(IOMode type)
{
    switch(type)
    {
    case IOSelect:
        poller_ = new SelectPoller();
        break;
    case IOAsyncSelect:
        poller_ = new AsyncSelectPoller();
        break;
    case IOEventSelect:
        poller_ = new AsyncEventPoller();
        break;
    default:
        CHECK(false) << "invalid multiplexer type: " << type;
    }
}

void EventLoop::AddEvent(SOCKET fd, int mask, EventProc func)
{
    assert(poller_ != NULL);
    auto iter = events_.find(fd);
    if (iter == events_.end())
    {
        EventEntry entry;
        entry.fd = fd;
        entry.mask =  mask;
        events_[fd] = entry;
    }
    EventEntry* entry = &events_[fd];
    if ((mask & EV_READABLE) || (mask & EV_WRITABLE))
    {
        entry->proc = func;
    }
    poller_->AddFd(fd, mask);
}

void EventLoop::DelEvent(SOCKET fd, int mask)
{
    auto iter = events_.find(fd);
    if (iter != events_.end())
    {
        EventEntry* entry = &iter->second;
        entry->mask = entry->mask & (~mask);
        if (entry->mask == EV_NONE)
        {
            events_.erase(fd);
        }
    }
}

EventEntry* EventLoop::GetEntry(SOCKET fd)
{
    auto iter = events_.find(fd);
    if (iter != events_.end())
    {
        return &iter->second;
    }
    return NULL;
}

void EventLoop::AddFiredEvent(SOCKET fd, int mask, int ec)
{
    FiredEvent event;
    event.fd = fd;
    event.mask = mask;
    event.ec = ec;
    fired_.push_back(event);
}

void EventLoop::Run()
{
    while (true)
    {
        RunOne();
    }
}

void EventLoop::RunOne()
{
    if (events_.empty())
    {
        return ;
    }

    fired_.clear();
    poller_->Poll(this, 100);
    for (unsigned i = 0; i < fired_.size(); i++)
    {
        const FiredEvent* event = &fired_[i];
        auto iter = events_.find(event->fd);
        if (iter == events_.end())
            continue;
        EventEntry* entry = &iter->second;
        if ((event->mask & EV_READABLE) || (event->mask & EV_WRITABLE))
        {
            entry->proc(event->fd, event->mask, event->ec);
        }
    }
}
