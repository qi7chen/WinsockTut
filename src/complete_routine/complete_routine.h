/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <WinSock2.h>

/*
 * A simple echo server implemented by alertable I/O
 */

/* initialize and release internal data */
int     comp_routine_init();
void    comp_routine_release();

/* create listen socket for accept */
SOCKET  create_acceptor(const char* host, int port);

/* run alertable event */
int     event_loop(SOCKET acceptor);
