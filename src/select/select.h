/**
 *  @file   select.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by select()
 *
 */

#pragma once

#include <Winsock2.h>

/* initialize and release internal data */
int     select_init();
void    select_release();

/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, const char* port);

/* main select loop */
int     select_loop(SOCKET acceptor);
