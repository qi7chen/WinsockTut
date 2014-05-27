/**
 *  @file   overlap.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by overlapped I/O
 *			
 */

#pragma once

#include <WinSock2.h>


/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, int port);

/* start event loop */
int     event_loop(SOCKET acceptor);

/* initialize and release internal data structure */
int     overlap_init();
void    overlap_release();
