// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <stdio.h>
#include <signal.h>
#include <string>
#include <memory>
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
    signal(SIGINT, HandleSignal);

    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <server/client> {host} {port} {mode}\n", argv[0]);
        return 1;
    }

    string type = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];
    PollerType mode = (PollerType)atoi(argv[4]);

    WinsockAutoInit init;

    PollerBase* poller = CreatePoller(mode);
    CHECK(poller != NULL);

    std::shared_ptr<EchoServer> server = NULL;
    std::shared_ptr<EchoClient> client = NULL;
    std::shared_ptr<IPollEvent> sink = NULL;
    if (type == "server")
    {
		server.reset(new EchoServer(poller));
        server->Start(host, port);
        sink = server;
        fprintf(stdout, "server started at %s:%s\n", host, port);
    }
    else if (type == "client")
    {
        client.reset(new EchoClient(poller));
        client->Start(host, port);
        sink = client;
        fprintf(stdout, "client started at %s:%s\n", host, port);
    }
    else
    {
        fprintf(stderr, "invalid instance type: [%s]\n", type.c_str());
    }

    while (!g_stop)
    {
        poller->Poll(50);
    }
    delete poller;

    return 0;
}
