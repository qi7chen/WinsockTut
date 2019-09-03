// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

class ChatServer
{
public:
    explicit ChatServer(IOServiceBase* service);
    ~ChatServer();

    void Start(const char* host, const char* port);
    void CloseSession(SOCKET fd);

    IOServiceBase* GetIOService() { return service_; }

private:
    void OnAccept(SOCKET fd);

private:
    IOServiceBase*  service_;
    SOCKET          acceptor_;
};
