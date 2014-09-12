/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "complete_routine.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


int main(int argc, const char* argv[])
{
    SOCKET acceptor;
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    comp_routine_init();
    acceptor = create_acceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    while (event_loop(acceptor))
        ;

    comp_routine_release();
    return 0;
}

