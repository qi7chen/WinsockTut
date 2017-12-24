// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <stdio.h>
#include <string>
#include "Common/WinsockInit.h"
#include "EchoServer.h"
#include "EchoClient.h"

using namespace std;


int main(int argc, const char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <server/client> <host> <port>\n");
        return 1;
    }
    
    WinsockAutoInit init;

    string type = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];
    IOMode mode = IOSelect;

    if (type == "server")
    {
        EchoServer server(mode);
        server.Start(host, port);
        server.Run();
    }
    else if (type == "client")
    {
        EchoClient client(mode);
        client.Start(host, port);
        client.Run();
    }
    else
    {
        fprintf(stderr, "invalid instance type: [%s]\n", type.c_str());
    }
    return 0;
}
