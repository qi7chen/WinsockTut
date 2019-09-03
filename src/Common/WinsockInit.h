// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include "Logging.h"
#include "WsaExt.h"

// Auto initialize winsock
struct WinsockAutoInit
{
    WinsockAutoInit()
    {
        WSADATA data;
        int r = WSAStartup(MAKEWORD(2, 2), &data);
        CHECK(r == 0);
        WsaExt init;    // initialize winsock extension 
    }

    ~WinsockAutoInit()
    {
        WSACleanup();
    }
};
