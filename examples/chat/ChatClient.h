// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

class ChatClient
{
public:
    explicit ChatClient(IOServiceBase* service);
    ~ChatClient();

    int Start(const char* host, const char* port);

private:
    int Connect(const char* host, const char* port);

    void OnConnect(int error);

private:
    IOServiceBase*  service_;
    SOCKET          fd_;
};
