/**
 *  @file   complete_routine.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by alertable I/O
 *
 */

#pragma once

#include <WinSock2.h>

/* initialize and release internal data */
int     comp_routine_init();
void    comp_routine_release();

/* create listen socket for accept */
SOCKET  create_acceptor(const char* host, int port);

/* run alertable event */
int     event_loop(SOCKET acceptor);
