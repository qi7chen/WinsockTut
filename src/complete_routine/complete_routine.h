/**
 *  @file   complete_routine.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by alertable I/O
 *			
 */

#pragma once

#include "../common/utility.h"


struct socket_data
{
    // socket handle
    SOCKET          socket_;

    // winsock buffer object
    WSABUF          wsabuf_;

    // data buffer
    char            databuf_[kDefaultBufferSize];

    // overlapped structure
    WSAOVERLAPPED   overlap_;
};



// Allocate associated data for each socket
socket_data* alloc_data(SOCKET sockfd);


// Free associated socket data
void    free_data(socket_data* data);


// Post an asynchounous recv request
bool    post_recv_request(socket_data* data);


SOCKET  create_listen_socket(const char* host, int port);