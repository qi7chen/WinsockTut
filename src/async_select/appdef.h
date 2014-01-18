/**
 *  @file   appdef.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  
 */

#pragma once

#include "../common/utility.h"


#define WM_SOCKET           WM_USER + 0xF0


// Create acceptor and associate to window message queue
bool InitializeServer(HWND hwnd,  const char* host, int port);

// Close all connection
void CloseServer();

// I/O operation handling
bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


