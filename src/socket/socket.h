/**
 *  @file   socket.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by BSD socket
 *          
 */

#pragma once

#include <WinSock2.h>


/* initialize and release internal data */
int     socket_init();
void    socket_release();

/* create a listen socket for accept */
SOCKET  create_acceptor(const char* host, const char* port);

/* main loop */
int     socket_loop(SOCKET sockfd);

