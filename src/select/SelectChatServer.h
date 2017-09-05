/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <WinSock2.h>
#include "common/Factory.h"
#include "common/ChatRoom.h"

// A simple chat server implemented by select I/O model
class SelectChatServer : public IChatServer
{
public:
    SelectChatServer();
    ~SelectChatServer();

    bool Init(const char* host, const char* port);

    int Run();

private:
    void OnNewConnection(SOCKET fd);
    bool Poll();    

private:
    SOCKET                  acceptor_;
    std::map<int, SOCKET>   identities_;
    std::map<SOCKET, int>   connections_;
    ChatRoom                room_;
};