// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

class ChatServer;

class ChatSession
{
public:
    ChatSession(ChatServer* server, SOCKET fd);
    ~ChatSession();

    void StartRead();

private:
    void OnRead(int error, int bytes);
    void OnWritten(int error, int bytes);

private:
    IOServiceBase*  service_;
    ChatServer*     server_;
    SOCKET          fd_;
};
