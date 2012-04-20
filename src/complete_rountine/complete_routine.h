
#pragma once


#include <WinSock2.h>
#include "../common/utility.h"




struct socket_data
{
    SOCKET          socket_;
    WSABUF          wsabuf_;
    char            databuf_[BUFE_SIZE];
    WSAOVERLAPPED   overlap_;
};



// allocate resources for a new connected socket
socket_data* alloc_data(SOCKET sockfd);



// close socket and free resource
void free_data(socket_data*& ptr);


// post a async recv request
bool post_recv_request(socket_data* data);


// callback routine after recv completed
void CALLBACK recv_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap, 
                            DWORD flags);


// callback routine after send completed
void CALLBACK send_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap, 
                            DWORD flags);