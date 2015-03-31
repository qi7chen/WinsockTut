/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <Winsock2.h>

/*
 * A simple echo server implemented by select()
 */

/* initialize and release internal data */
int     select_init();
void    select_release();

/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, const char* port);

/* main select loop */
int     select_loop(SOCKET acceptor);
