/**
 *  @file:   overlapped.h
 *  @brief:  Overlapped I/O use event
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include <tchar.h>
#include <WinSock2.h>

class overlap_server
{
public:
    overlap_server();

    bool    start(const TCHAR* host, short port);

    void                push_event_data(PER_HANDLE_DATA*);
    PER_HANDLE_DATA*    pop_event_data();

private:
    overlap_server(const overlap_server&);
    overlap_server& operator = (const overlap_server&);


    bool    alloc_handle_data(SOCKET socket);
    void    free_handle_data(PER_HANDLE_DATA* handle_data);


private:
    SOCKET          listen_socket_;
    spinlock        mutex_;
    std::queue<PER_HANDLE_DATA*>        events_queue_;
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;
};


