/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <WinSock2.h>
#include "EchoServer.h"
#include "common/utility.h"

int main(int argc, const char* argv[])
{
    int r = 0;
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    WSADATA data;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);

    r = StartEchoServer(host, port);
    if (r < 0)
    {
        fprintf(stderr, "start echo server failed.\n");
    }

    WSACleanup();
    return r;
}

