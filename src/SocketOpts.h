// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <WinSock2.h>

// Create an acceptor socket file descriptor
SOCKET CreateTCPAcceptor(const char* host, const char* port);

// set socket to non-blocking mode
int SetNonblock(SOCKET fd, bool nonblock);
