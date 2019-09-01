// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include "Logging.h"

// Auto initialize winsock
struct WinsockAutoInit
{
    WinsockAutoInit()
    {
        WSADATA data;
        int r = WSAStartup(MAKEWORD(2, 2), &data);
        CHECK(r == 0);
    }

    ~WinsockAutoInit()
    {
        WSACleanup();
    }
};
