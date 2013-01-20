/**
*  @file   woker.h
*  @author ichenq@gmail.com
*  @date   Oct 19, 2011
*  @brief  worker thread abstraction, for event dispatching
*/

#pragma once


#include "../common/utility.h"


// thread message
const unsigned WM_ADD_NEW_SOCKET = WM_USER + 0xff;


//
// event thread worker
//
class worker
{
public:
    worker();
    ~worker();

    // called by main thread
    void start();

    // 
    unsigned get_id() const {return my_thread_;} ;

    bool    is_full();

    void    main_loop();

private:
    worker(const worker&);
    worker& operator = (const worker&);

   
    int  event_handler(SOCKET sockfd, const WSANETWORKEVENTS* events);

    bool on_recv(SOCKET sockfd, int error);
    bool on_write(SOCKET sockfd, int error);
    bool on_close(SOCKET sockfd, int error);


    // handle thread messages
    bool handle_messages();

    // nonlock
    void reset();


private:
    WSAEVENT    eventlist_[WSA_MAXIMUM_WAIT_EVENTS + 1]; // for convenience of deleting
    SOCKET      socklist_[WSA_MAXIMUM_WAIT_EVENTS + 1];
    size_t      count_;

    unsigned    my_thread_;
};

