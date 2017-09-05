/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <Windows.h>
#include <WinSock2.h>
#include "common/Factory.h"



// A simple echo server implemented by WSAAsyncSelect()
class AsyncSelectChatServer : public IChatServer
{
public:
    AsyncSelectChatServer();
    ~AsyncSelectChatServer();

    bool Init(const char* host, const char* port);

    int Run();

private:
    HWND CreateHidenWindow();
    void HandleSocketEvent(SOCKET fd, int event, int error);
    void HandleAccept(SOCKET fd);

private:
    HWND    hWnd_;
    SOCKET  acceptor_;
};
