// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>

// set socket to non-blocking mode
int SetNonblock(SOCKET fd, bool nonblock);

int BindAnyAddr(SOCKET fd, int family);

// enable previously set properties or options after ConnectEx
int UpdateConnectCtx(SOCKET fd);

// make sure 'size' is read before to return (unless error is encountered)
int ReadSome(SOCKET fd, void* buf, int size);

// make sure 'size' is written before to return (unless error is encountered)
int WriteSome(SOCKET fd, const void* buf, int size);

bool IsSelfConnection(SOCKET fd);

SOCKET CreateTCPAcceptor(const char* host, const char* port, bool nonblock = true, bool ipv6 = false);

SOCKET CreateTcpConnector(const char* host, const char* port, bool nonblock = true, bool ipv6 = false);
