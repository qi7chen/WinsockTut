// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <assert.h>
#include <string>
#include "Common/WinsockInit.h"
#include "Proactor/OverlappedIOService.h"
#include "Proactor/WsaExt.h"
#include "Common/Any.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <server/client> <mode> <host> <port>\n", argv[0]);
        return 1;
    }

    string type = argv[1];
    string host = argv[2];
    string port = argv[3];
    IOServiceType mode = (IOServiceType)atoi(argv[4]);

    WinsockAutoInit init;
    WsaExt::Init(INVALID_SOCKET);

    IOServiceBase* service = CreateIOService(mode);
    assert(service != nullptr);

    service->AsyncConnect(host, port, [=](int ec)
    {
        LOG(INFO) << ec << ", " << fd;
    });

    while(true)
    {
        service->Poll(50);
    }

    return 0;
}
