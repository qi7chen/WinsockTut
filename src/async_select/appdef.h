/*
 *  @file:  appdef.h
 *  @brief: 
 *
 */
#pragma once

#include "../common/utility.h"


#define WM_SOCKET           WM_USER + 0xF0

#define WM_TEXT_UPDATE      WM_USER + 0xFF



// initialize WSAAsynSelect server
bool InitializeServer(HWND hwnd, const _tstring& strHost, const _tstring& strPort);

// 
void CloseServer();

// socket events handler
bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


// append text to edit control
bool AppendLogText(const TCHAR* text, int len);
