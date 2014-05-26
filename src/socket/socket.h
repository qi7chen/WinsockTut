/**
 *  @file   socket.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by basic BSD socket API
 *			one thread per connection
 */

#pragma once

#include <WinSock2.h>

SOCKET  create_acceptor(const char* host, const char* port);
int     on_accept(SOCKET sockfd);