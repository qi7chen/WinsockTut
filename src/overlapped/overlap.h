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
 * A simple echo server implemented by overlapped I/O
 */

/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, int port);

/* start event loop */
int     event_loop(SOCKET acceptor);

/* initialize and release internal data structure */
int     overlap_init();
void    overlap_release();
