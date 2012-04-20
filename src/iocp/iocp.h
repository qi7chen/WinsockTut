/**
 *  @file:   iocp.h
 *  @brief:  a simple echo server use I/O completion port
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once


#pragma warning(push)
#pragma warning(disable: 4324)


#include <WinSock2.h>
#include <map>
#include <list>
#include <queue>
#include <memory>
#include "../common/utility.h"
#include "../common/thread.h"
#include "../common/mutex.h"



class iocp_server
{
public:
    iocp_server();
    ~iocp_server();

    bool    start(const TCHAR* host, short port);

    HANDLE  completion_port() {return completion_port_;}

    // called by worker thread(s)
    void    push_command(PER_HANDLE_DATA* handle_data);    

private:
    iocp_server(const iocp_server&);
    iocp_server& operator = (const iocp_server&);

    // events handlers
    void    on_accepted(PER_HANDLE_DATA* handle_data);
    void    on_recv(PER_HANDLE_DATA* handle_data);
    void    on_sent(PER_HANDLE_DATA* handle_data);
    void    on_disconnect(PER_HANDLE_DATA* handle_data);
    void    on_closed(PER_HANDLE_DATA* handle_data);
    
    // main loop
    void        wait_loop();

    bool        create_workers(DWORD concurrency = 0);
    bool        create_completion_port(unsigned concurrency);    
    bool        create_listen_socket(const sockaddr_in& addr);
    bool        post_an_accept(); 

    PER_HANDLE_DATA*    pop_command();
    

    // resource management
    PER_HANDLE_DATA*    alloc_socket_handle();
    void                free_socket_handle(PER_HANDLE_DATA* handle_data);

private:
    typedef scoped_lock<mutex>  auto_lock;

    SOCKET      listen_socket_;
    HANDLE      completion_port_;
    mutex       mutex_;

    std::vector<shared_ptr<thread>>         workers_;       // worker thread(s)
    std::queue<PER_HANDLE_DATA*>            command_queue_; // commands queue for worker thread(s) to push
    std::list<PER_HANDLE_DATA*>             free_list_;     // handles to be reused
    std::map<SOCKET, PER_HANDLE_DATA*>      info_map_;      // handles in use
};


#pragma warning(pop)