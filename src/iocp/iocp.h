//  I/O Completion Port manager
//  by ichenq@gmail.com at Oct 19, 2011

#pragma once


#pragma warning(push)
#pragma warning(disable: 4324)


#include <WinSock2.h>
#include <map>
#include <list>
#include <queue>
#include <memory>
#include "../common/utility.h"
#include "../common/mutex.h"



class iocp_server
{
public:
    iocp_server();
    ~iocp_server();

    bool    start(const TCHAR* host, const TCHAR* port);

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
    void    wait_loop();

    bool    create_workers(DWORD concurrency);   
    bool    create_listen_socket(const _tstring& host, const _tstring& port);
    bool    post_an_accept(); 

    PER_HANDLE_DATA*    pop_command();
    

    // resource management
    PER_HANDLE_DATA*    alloc_socket_handle();
    void                free_socket_handle(PER_HANDLE_DATA* handle_data);

private:
    typedef scoped_lock<mutex>  auto_lock;

    // listen socket descriptor
    SOCKET      listen_socket_;

    // completion port handle
    HANDLE      completion_port_;

    // mutex object
    mutex       mutex_;

    // worker thread(s)
    std::vector<unsigned>                   workers_;

    // commands queue for worker thread(s) to push
    std::queue<PER_HANDLE_DATA*>            command_queue_; 

    // handles to be reused
    std::list<PER_HANDLE_DATA*>             free_list_;

    // handles in use
    std::map<SOCKET, PER_HANDLE_DATA*>      info_map_;
};


#pragma warning(pop)