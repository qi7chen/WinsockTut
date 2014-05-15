/**
 *  @file   select.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by select()
 *			
 */

#pragma once

#include <Winsock2.h>

SOCKET  create_listen_socket(const char* host, const char* port);

bool    select_loop(SOCKET acceptor);
