/*
 *  @file:  appdef.h
 *  @brief: 
 *
 */
#pragma once

#include "../common/utility.h"


#define WM_SOCKET           WM_USER + 0xF0


// 初始化监听套接字并将网络事件关联到窗口消息
bool InitializeServer(HWND hwnd,  const char* host, int port);

// 关闭所有连接
void CloseServer();

// 处理套接字消息
bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error);


