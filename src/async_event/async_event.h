/**
 *  @file   async_select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by WSAEventSelect()
 *			
 */

#include <WinSock2.h>


SOCKET  create_listen_socket(const char* host, int port);

bool    on_accept(SOCKET sockfd);

bool    event_loop();
