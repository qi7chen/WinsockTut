/**
 *  @file   iocp.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by I/O completion port
 * 
 */

#pragma once

#include <map>
#include <list>
#include <queue>
#include <memory>
#include <atlsync.h>
#include "../common/utility.h"



class IOCPServer
{
public:
    IOCPServer();
    ~IOCPServer();

    // start server
    bool    start(const char* host, int port);
    
    HANDLE  completion_port() {return completion_port_;}

    // push I/O operation to command queue
    void    push_command(PER_HANDLE_DATA* handle_data);    

private:
    IOCPServer(const IOCPServer&);
    IOCPServer& operator = (const IOCPServer&);

    // event handling
    void    on_accepted(PER_HANDLE_DATA* handle_data);
    void    on_recv(PER_HANDLE_DATA* handle_data);
    void    on_sent(PER_HANDLE_DATA* handle_data);
    void    on_disconnect(PER_HANDLE_DATA* handle_data);
    void    on_closed(PER_HANDLE_DATA* handle_data);
    
    bool    event_loop();

    // create worker routine
    bool    create_workers(DWORD concurrency);   

    // create acceptor
    bool    create_listen_socket(const char* host, int port);

    // Post an asynchounous accept request
    bool    post_an_accept(); 

    // Pop a I/O command from queue
    PER_HANDLE_DATA*    pop_command();
    
    // Socket resource management
    PER_HANDLE_DATA*    alloc_socket_handle();
    void                free_socket_handle(PER_HANDLE_DATA* handle_data);

private:    
    SOCKET      listen_socket_; 
    HANDLE      completion_port_;
           
    std::vector<unsigned>   workers_;
    
    ATL::CCriticalSection   mutex_;
    std::queue<PER_HANDLE_DATA*>    command_queue_;   // I/O operation queue
    
    std::list<PER_HANDLE_DATA*>         free_list_;
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;
};
