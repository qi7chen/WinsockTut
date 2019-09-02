// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>

// set socket to non-blocking mode
int SetNonblock(SOCKET fd, bool nonblock);

int BindAnyAddr(SOCKET fd, int family);

// read no more than `size` bytes
int ReadSome(SOCKET fd, void* buf, int size);

// write `size` bytes 
int WriteSome(SOCKET fd, const void* buf, int size);

bool IsSelfConnection(SOCKET fd);

SOCKET CreateTCPAcceptor(const char* host, const char* port, bool nonblock = true, bool ipv6 = false);

SOCKET CreateTcpConnector(const char* host, const char* port, bool nonblock = true, bool ipv6 = false);
