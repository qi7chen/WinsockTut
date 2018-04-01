// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>
#include <functional>

// set socket to non-blocking mode
int SetNonblock(SOCKET fd, bool nonblock);

// read no more than `size` bytes
int ReadSome(SOCKET fd, void* buf, int size);

// write `size` bytes 
int WriteSome(SOCKET fd, const void* buf, int size);

typedef std::function<bool(addrinfo*)> LoopProcessor;

void LoopThroughStreamAddr(const char* host, const char* port, LoopProcessor processor);
