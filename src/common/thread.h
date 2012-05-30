/**
*  @file:   thread.h
*  @brief:  a simple thread class
*
*  @author: ichenq@gmail.com
*  @date:   Oct 19, 2011
*/

#pragma once

#include "utility.h"
#include <Windows.h>
#include <memory>



struct thread_data_base
{
    HANDLE      thread_handle_;
    unsigned    thread_id_;

    thread_data_base();
    virtual ~thread_data_base();

    virtual void run() = 0;    
};




template <typename F>
class thread_data : public thread_data_base
{
public:
    explicit thread_data(F f)
        : f_(f)
    {}

    void run()
    {
        f_();
    }

private:
    F       f_;
};


bool start_thread(thread_data_base* data);


template <typename F>
unsigned create_thread(F f)
{
    thread_data_base* pbase = NULL;
    try
    {
        pbase = new thread_data<F>(f);
    }
    catch (std::bad_alloc&)
    {
        LOG_ERROR(_T("allocate new thread failed"));
        return 0;
    }
    if (start_thread(pbase))
    {
        return (unsigned)pbase->thread_handle_;
    }
    return 0;
}
