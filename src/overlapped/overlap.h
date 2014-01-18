/**
 *  @file   overlap.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by overlapped I/O
 *			
 */

#pragma once

#include "../common/utility.h"


// New connection arrival
bool    on_accept(SOCKET sockfd);

// Start event loop
bool    event_loop();
