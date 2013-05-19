/*
 *  @file:  appdef.h
 *  @brief: 
 *
 */
#pragma once

#include "../common/utility.h"


#define WM_SOCKET           WM_USER + 0xF0

#define WM_TEXT_UPDATE      WM_USER + 0xFF



// Initialize WSAAsynSelect server
bool InitializeServer(HWND hwnd, const _tstring& strHost, const _tstring& strPort);

// Release socket resouce
void CloseServer();

// Socket events handler
bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


