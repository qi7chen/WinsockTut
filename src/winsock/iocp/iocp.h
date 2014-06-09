/**
 *  @file   iocp.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by I/O completion port
 * 
 */

#pragma once

#include "common/utility.h"


/* initialize iocp server internal data structures */
int     server_init(const char* host, short port);
void    server_destroy();

/* start run server */
int     server_run();
