// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"
#include <unordered_map>

class Session;

class Server
{
public:
    explicit Server(IOServiceBase* service);
    ~Server();

    void Start(const char* host, const char* port);
    void CloseSession(int sid);

    IOServiceBase* GetIOService() { return service_; }

private:
    void Cleanup();
    void StartAccept();
    void OnAccept(OverlapContext* ctx);

private:
    IOServiceBase*  service_;
    OverlapContext* ctx_;
    SOCKET          acceptor_;
    int             counter_;
    std::unordered_map<int, Session*> sessions_;
};
