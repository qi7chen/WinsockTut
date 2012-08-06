/**
 *  @file:   worker.h
 *  @brief: 
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once

#include "../common/config.h"
#include "../common/utility.h"



enum 
{
    WM_NEW_SOCKET = WM_USER + 0xf0,
    WM_THREAD_LIMIT,
};


static const unsigned MAX_COUNT = WSA_MAXIMUM_WAIT_EVENTS;
static const unsigned MAX_MS = 50;


struct tagSocketData
{
    unsigned total_count;
    WSAEVENT eventList[MAX_COUNT];
    PER_HANDLE_DATA* dataList[MAX_COUNT];
};


void worker_routine();


