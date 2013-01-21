//  Overlapped I/O worker thread
//  by ichenq@gmail.com at Oct 19, 2011


#pragma once

#include "../common/utility.h"


enum 
{
    WM_NEW_SOCKET = WM_USER + 0xf0,
    WM_THREAD_LIMIT,
};

// max socket per thread
const unsigned MAX_COUNT = WSA_MAXIMUM_WAIT_EVENTS;

// max timeout millisecond
const unsigned MAX_TIMEOUT = 50;


struct socket_data
{
    unsigned            total_count;
    WSAEVENT            eventList[MAX_COUNT];
    PER_HANDLE_DATA*    dataList[MAX_COUNT];
};


void worker_routine();


