/**
 *  @file   overlap.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by overlapped I/O
 *			
 */

#pragma once

#include <WinSock2.h>


// new connection arrival
bool    on_accept(SOCKET sockfd);

// start event loop
bool    event_loop();

// create acceptor
SOCKET create_listen_socket(const char* host, int port);