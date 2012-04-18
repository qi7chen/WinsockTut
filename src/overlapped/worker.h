/**
 *  @file:   worker.h
 *  @brief: 
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once

#include "../common/utility.h"
#include "../common/mutex.h"
#include "../common/thread.h"


class overlap_server;

class worker
{
public:
    worker();

    void start(overlap_server* server);

private:
    worker(const worker&);
    worker& operator = (const worker&);

    void  wait_loop(overlap_server* server);

private:
    spinlock    mutex_;
    int         total_count_;
    WSAEVENT    event_list_[WSA_MAXIMUM_WAIT_EVENTS];
    std::shared_ptr<thread> work_thread_;
};
