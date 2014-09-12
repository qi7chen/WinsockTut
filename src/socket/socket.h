/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <WinSock2.h>

/*
 * A simple echo server implemented by BSD socket
 */

/* initialize and release internal data */
int     socket_init();
void    socket_release();

/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, const char* port);

/* main loop */
int     socket_loop(SOCKET sockfd);

