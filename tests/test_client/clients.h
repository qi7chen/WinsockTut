/**
 *  @file    clients.h
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 *  @brief:  Echo clients implemented by I/O completion port
 */

#pragma once

#include "common/utility.h"
#include <map>


class clients
{
public:
    clients();
    ~clients();

    // Start all echo clients
    bool start(const char* host, short port, int count);

private:
    clients(const clients&);
    clients& operator = (const clients&);

    // I/O operation handling
    void        on_connected(PER_HANDLE_DATA* handle_data);
    void        on_recv(PER_HANDLE_DATA* handle_data);
    void        on_close(PER_HANDLE_DATA* handle_data);
    void        after_sent(PER_HANDLE_DATA* handle_data);

    // Create a connection
    bool                create_one_client(const sockaddr_in& addr);

    // Connection resource management
    PER_HANDLE_DATA*    alloc_handle_data(SOCKET sock_fd);
    void                free_handle_data(PER_HANDLE_DATA* handle_data);

private:
    HANDLE              comletport_handle_;    
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;
};