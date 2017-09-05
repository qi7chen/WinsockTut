/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <Windows.h>
#include <WinSock2.h>
#include <map>
#include "common/Factory.h"

// A simple echo server implemented by WSAEventSelect()
class AsyncEventChatServer
{
public:
    AsyncEventChatServer();
    ~AsyncEventChatServer();

    //
    bool Init(const char* host, const char* port);

    //
    int Run();

private:
    int Poll();
    void HandleEvents(SOCKET fd, WSANETWORKEVENTS* event);
    void HandleAccept(SOCKET fd);
    

private:
    SOCKET                      acceptor_;
    std::map<SOCKET, WSAEVENT>  connections_;
    std::map<WSAEVENT, SOCKET>  events_;
};