/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <WinSock2.h>
#include "common/Factory.h"


struct ConnetionData
{
    WSAOVERLAPPED   overlap;
    SOCKET          fd;         // file descriptor
    WSABUF          buf;        // WSA buffer object
    char            data[1];    // dynamic recv buffer
};

// A simple chat server implemented by alertable I/O
class ComRoutineChatServer : public IChatServer
{
public:
    ComRoutineChatServer();
    ~ComRoutineChatServer();

    bool Init(const char* host, const char* port);

    int Run();

private:
    static void CALLBACK OnRecvComplete(DWORD error,
                                        DWORD bytes_transferred,
                                        WSAOVERLAPPED* overlap,
                                        DWORD flags);

    static void CALLBACK OnSendComplete(DWORD error,
                                        DWORD bytes_transferred,
                                        WSAOVERLAPPED* overlap,
                                        DWORD flags);

    int Poll();

private:
    SOCKET  acceptor_;
};
