/**
 *  @file   appdef.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  
 */

#pragma once

#include <WinSock2.h>


#define WM_SOCKET           WM_USER + 0xF0


// Create acceptor and associate to window message queue
int     InitializeServer(HWND hwnd,  const char* host, int port);

// Close all connection
void    CloseServer();

// I/O operation handling
int     HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


