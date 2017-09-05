/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include "common/Factory.h"

// A simple chat server implemented by overlapped I/O
class OverlapChatServer : public IChatServer
{
public:
    OverlapChatServer();
    ~OverlapChatServer();

    bool Init(const char* host, const char* port);

    int Run();

private:


};
