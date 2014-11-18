/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdlib.h>
#include "clients.h"
#include "common/utility.h"


int main(int argc, char* argv[])
{
    int timeout = 500;
    const char* default_host = DEFAULT_HOST;
    short default_port = atoi(DEFAULT_PORT);
    int default_count = 2000;

    if (argc == 4)
    {
        default_host = argv[1];
        default_port = atoi(argv[2]);
        default_count = atoi(argv[3]);
    }

    if (loop_init())
    {
        create_connections(default_host, default_port, default_count);
        while (loop_run(timeout))
            ;
    }
    loop_destroy();

    return 0;
}