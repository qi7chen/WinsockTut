// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <signal.h>
#include <string>
#include <iostream>
#include <memory>
#include "Common/WinsockInit.h"
#include "IOServiceBase.h"
#include "Server.h"
#include "Client.h"

using namespace std;

IOServiceBase* service = nullptr;


bool g_stop = false;

void handleSignal(int sig)
{
    g_stop = true;
    fprintf(stderr, "stop io service\n");
}

int main(int argc, char* argv[])
{
    signal(SIGINT, handleSignal);
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <server/client> <mode> <host> <port> <i/o model>\n", argv[0]);
        return 1;
    }

    string type = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];
    IOServiceType mode = (IOServiceType)atoi(argv[4]);

    WinsockAutoInit init;

    IOServiceBase* service = CreateIOService(mode);
    CHECK(service != nullptr);

    std::shared_ptr<Server> server = NULL;
    std::shared_ptr<Client> client = NULL;
    if (type == "server")
    {
        server.reset(new Server(service));
        server->Start(host, port);
        fprintf(stdout, "server started at %s:%s\n", host, port);
    }
    else if (type == "client")
    {
        client.reset(new Client(service));
        client->Start(host, port);
        fprintf(stdout, "client started at %s:%s\n", host, port);
    }
    else
    {
        fprintf(stderr, "invalid instance type: [%s]\n", type.c_str());
    }

    while(!g_stop)
    {
        service->Run(50);
    }
    delete service;

    return 0;
}
