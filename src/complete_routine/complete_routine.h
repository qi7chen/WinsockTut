/**
 *  @file   complete_routine.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用完成例程模型实现的简单Echo Server
 *			
 */

#pragma once

#include "../common/utility.h"


// 套接字关联数据
struct socket_data
{
    SOCKET          socket_;
    WSABUF          wsabuf_;
    char            databuf_[kDefaultBufferSize];
    WSAOVERLAPPED   overlap_;
};



// 为新连接获取相关资源
socket_data* alloc_data(SOCKET sockfd);


// 释放连接所占用的资源
void free_data(socket_data* data);


// 发起读取数据请求(异步)
bool post_recv_request(socket_data* data);

