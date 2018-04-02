// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include <assert.h>
#include <string>
#include <iostream>
#include "Common/WinsockInit.h"
#include "Common/Error.h"
#include "Common/Any.h"
#include "Proactor/OverlappedIOService.h"
#include "WsaExt.h"

using namespace std;

IOServiceBase* service = nullptr;

void OnConnected(OverlapFd* fd)
{
    if (fd->err > 0)
    {
        cout << GetErrorMessage(fd->err) << endl;
        return ;
    }
    char msg[] = "a quick brown fox jumps over the lazy dog";
    service->AsyncWrite(fd, msg, sizeof(msg), [](int ec, int nbytes)
    {
        cout << ec << ": " << nbytes;
    });
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <server/client> <mode> <host> <port>\n", argv[0]);
        return 1;
    }

    string type = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];
    IOServiceType mode = (IOServiceType)atoi(argv[4]);

    WinsockAutoInit init;
    WsaExt::Init(INVALID_SOCKET);

    service = CreateIOService(mode);
    assert(service != nullptr);

    service->AsyncConnect(host, port, OnConnected);

    while(true)
    {
        service->Poll(50);
    }
    delete service;

    return 0;
}
