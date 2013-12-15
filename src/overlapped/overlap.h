/**
 *  @file   overlap.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用重叠I/O模型实现的简单Echo Server
 *			
 */

#pragma once

#include "../common/utility.h"

// 处理连接上的客户端套接字
bool    on_accept(SOCKET sockfd);

// 事件循环
bool    event_loop();
