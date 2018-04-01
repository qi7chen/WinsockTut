// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <functional>

enum OperationType
{
    OpConnect = 1,
    OpAccept = 2,
    OpRead = 3, 
    OpWrite = 4,
    OpClose = 5,
};

typedef std::function<void(int)> ConnectCallback;
typedef std::function<void(int)> AcceptCallback;
typedef std::function<void(int,int)> ReadCallback;
typedef std::function<void(int,int)> WriteCallback;
typedef std::function<void()> TimerCallback;

