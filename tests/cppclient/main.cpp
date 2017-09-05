/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "EchoClient.h"
#include "common/utility.h"


int main(int argc, char* argv[])
{
    int r = 0;
    const char* host = "127.0.0.1";
    const char* port = DEFAULT_PORT;
    int count = 2000;
    WSADATA data;
    if (argc >= 4)
    {
        host = argv[1];
        port = argv[2];
        count = atoi(argv[3]);
    }

    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);

    r = StartEchoClient(count, host, port);
    if (r < 0)
    {
        fprintf(stderr, "start echo client failed.\n");
    }

    WSACleanup();
    return r;
}