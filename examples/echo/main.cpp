// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string>
#include "Common/WinsockInit.h"
#include "EchoServer.h"
#include "EchoClient.h"

using namespace std;

bool g_stop = false;

void HandleSignal(int sig)
{
    g_stop = true;
    fprintf(stderr, "stop poller\n");
}

int main(int argc, const char* argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <server/client> <mode> <host> <port>\n", argv[0]);
        return 1;
    }

    string type = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];
    PollerType mode = (PollerType)atoi(argv[4]);

    WinsockAutoInit init;
    PollerBase* poller = CreatePoller(mode);
    assert(poller != nullptr);

    if (type == "server")
    {
        EchoServer server(poller);
        server.Start(host, port);
        fprintf(stdout, "server started at %s:%s\n", host, port);
    }
    else if (type == "client")
    {
        EchoClient client(poller);
        client.Start(host, port);
        fprintf(stdout, "client started at %s:%s\n", host, port);
    }
    else
    {
        fprintf(stderr, "invalid instance type: [%s]\n", type.c_str());
    }

    signal(SIGINT, HandleSignal);
    while (!g_stop)
    {
        poller->Poll(50);
    }

    return 0;
}
