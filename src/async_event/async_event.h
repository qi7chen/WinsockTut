/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <WinSock2.h>

/*
 * A simple echo server implemented by WSAEventSelect()
 */

/* init and release internal data */
int     async_event_init();
void    async_event_release();

/* create accept listen socket */
SOCKET  create_acceptor(const char* host, int port);

/* select event loop */
int     event_loop();
