/**
*  @file:   woker.h
*  @brief:  thread abstraction, for event dispatching
*  @author: ichenq@gmail.com
*  @date:   Oct 19, 2011
*/

#pragma once


#include "../common/utility.h"
#include "../common/mutex.h"
#include "../common/thread.h"


//
// event thread worker
//
class worker
{
public:
    worker();
    ~worker();

    // should be called by manager thread
    void start();

    // push one socket to this worker
    bool push_socket(SOCKET sockfd);

    // when worker is full, you cannot push socket to it
    bool full() ;

private:
    worker(const worker&);
    worker& operator = (const worker&);

    void main_loop();
    int  event_handler(SOCKET sockfd, const WSANETWORKEVENTS* events);

    bool on_recv(SOCKET sockfd, int error);
    bool on_write(SOCKET sockfd, int error);
    bool on_close(SOCKET sockfd, int error);

    // nonlock
    void reset();

private:
    WSAEVENT    eventlist_[WSA_MAXIMUM_WAIT_EVENTS + 1]; // for convenience of deleting
    SOCKET      socklist_[WSA_MAXIMUM_WAIT_EVENTS + 1];
    size_t      count_;

    shared_ptr<thread> thrd_ptr_;
    spinlock    mutex_;

    typedef scoped_lock<spinlock> autolock;
};

