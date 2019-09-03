// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "PollerBase.h"
#include <stdint.h>
#include <Windows.h>
#include "SelectPoller.h"
#include "AsyncSelectPoller.h"
#include "AsyncEventPoller.h"


PollerBase::PollerBase()
    : last_err_(0)
{
}

PollerBase::~PollerBase()
{
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
