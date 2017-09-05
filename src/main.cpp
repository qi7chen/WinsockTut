/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <typeinfo>
#include <stdexcept>
#include "common/Utils.h"
#include "common/Factory.h"


int main(int argc, const char* argv[])
{
    int mode = 1;
    const char* host = "0.0.0.0";
    const char* port = "9527";
    if (argc >= 4)
    {
        mode = atoi(argv[1]);
        host = argv[2];
        port = argv[3];
    }
    else 
    {
        fprintf(stderr, "Warning: Usage: %s [mode] [host] [port]\n", argv[0]);
    }

    WinsockAutoInit init;
    
    IChatServer* server = CreateChatServer(mode);
    if (server == NULL)
    {
        fprintf(stderr, "cannot create server with mode %d\n", mode);
        return 0;
    }
    try
    {
        if (server->Init(host, port))
        {
            return server->Run();
        }
        else
        {
            fprintf(stderr, "init server failed\n");
            return 1;
        }
    }
    catch (std::exception& ex)
    {
        fprintf(stderr, "encoutered exception %s: %s", typeid(ex).name(), ex.what());
        return 1;
    }
}