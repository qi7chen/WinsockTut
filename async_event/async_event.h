/**
 *  @file:   async_event.h
 *  @brief:  a simple echo server using asynchronous event
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once


#include "../common/utility.h"
#include <WinSock2.h>



class async_event_server
{
public:
    async_event_server();

    bool    start(const TCHAR* host, short port);


private:
    async_event_server(const async_event_server&);
    async_event_server& operator = (const async_event_server&);

    // create the listening socket
    bool    create_listen_socket(const TCHAR* host, short port);

    // associate events and buffer resource to a socket
    bool    dispatch_event(SOCKET socket, int events);

    // free events and buffer resource assicated to socket
    bool    destroy_event(SOCKET socket);


    void    on_accepted(SOCKET socket, int error_code);
    void    on_recv(SOCKET socket, char* buffer, int error_code);
    void    do_send(SOCKET socket, char* buffer, int msglen, int error_code);
    void    on_close(SOCKET socket, int error_code);
    

private:    
    SOCKET      listen_socket_;
    unsigned    total_events_;
    WSAEVENT    event_list_[WSA_MAXIMUM_WAIT_EVENTS+1];
    SOCKET      socket_list_[WSA_MAXIMUM_WAIT_EVENTS+1];
    char*       buffer_list_[WSA_MAXIMUM_WAIT_EVENTS+1];
};


