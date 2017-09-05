/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

enum 
{
    MODE_SELECT = 1,
    MODE_ASYNC_SELECT = 2,
    MODE_ASYNC_EVENT = 3,
    MODE_COM_ROUTINE = 4,
    MODE_OVERLAPPED = 5,
    MODE_IOCP = 6,
};


struct IChatServer
{
    // Initialization
    virtual bool Init(const char* host, const char* port) = 0;

    // Run server in block mode
    virtual int Run() = 0;
};

IChatServer* CreateChatServer(int mode);
