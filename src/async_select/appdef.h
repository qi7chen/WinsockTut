/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <WinSock2.h>

/* 
 * A simple echo server implemented by WSAAsyncSelect()
 */

#define WM_SOCKET           WM_USER + 0xF0


/* create acceptor and associate to window message queue */
int     InitializeServer(HWND hwnd,  const char* host, int port);

/* close all connection */
void    CloseServer();

/* I/O operation handling */
int     HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


