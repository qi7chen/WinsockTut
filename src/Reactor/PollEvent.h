// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

struct IPollEvent
{
    virtual ~IPollEvent(){}
    virtual void OnReadable() = 0;
    virtual void OnWritable() = 0;
    virtual void OnTimeout() = 0;
};
