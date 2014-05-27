/**
 *  @file   async_select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by WSAEventSelect()
 *
 */

#include <WinSock2.h>

/* init and release internal data */
int     async_event_init();
void    async_event_release();

/* create accept listen socket */
SOCKET  create_acceptor(const char* host, int port);

/* select event loop */
int     event_loop();
