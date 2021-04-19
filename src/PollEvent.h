// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

struct ITimerEvent
{
    virtual ~ITimerEvent() {}

    virtual void OnTimeout() = 0;
};

struct IPollEvent : ITimerEvent
{
    virtual ~IPollEvent(){}

    virtual void OnReadable() = 0;
    virtual void OnWritable() = 0;
};
