/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "select.h"
#include "common/utility.h"

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

    CHECK(select_init());

    acceptor = create_acceptor(host, port);
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    while (select_loop(acceptor))
    {
    }

    select_release();

    return 0;
}

