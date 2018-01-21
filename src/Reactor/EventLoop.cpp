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

int EventLoop::AddEvent(SOCKET fd, EventProc func)
{
    assert(poller_ != NULL);
    EventEntry* entry = NULL;
    auto iter = events_.find(fd);
    if (iter == events_.end())
    {
        entry = new EventEntry();
        events_[fd] = entry;
    }
    else
    {
        entry = iter->second;
    }
    entry->fd = fd;
    entry->callback = func;
    
    int r = poller_->AddFd(fd);
    if (r < 0)
    {
        events_.erase(fd);
    }
    return r;
}

void EventLoop::DelEvent(SOCKET fd)
{
    auto iter = events_.find(fd);
    if (iter == events_.end())
    {
        LOG(ERROR) << "event entry not found " << fd;
        return;
    }
    events_.erase(iter);
    poller_->DeleteFd(fd);

}

EventEntry* EventLoop::GetEntry(SOCKET fd)
{
    auto iter = events_.find(fd);
    if (iter != events_.end())
    {
        return iter->second;
    }
    return NULL;
}

void EventLoop::AddFiredEvent(SOCKET fd, int ev, int err)
{
    FiredEvent event;
    event.fd = fd;
    event.event = ev;
    event.err = err;
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
    poller_->Poll(this, timeout_);
    for (unsigned i = 0; i < fired_.size(); i++)
    {
        const FiredEvent* event = &fired_[i];
        auto iter = events_.find(event->fd);
        if (iter == events_.end())
        {
            continue;
        }
        EventEntry* entry = iter->second;
        if (entry->callback)
        {
            entry->callback(event->fd, event->event, event->err);
        }
    }
}
