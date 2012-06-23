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



struct thread_data_base;



// start a thread
void start_thread(thread_data_base* data);

// send message
bool send_message_to(unsigned thread_id, 
                         unsigned msg, 
                         UINT_PTR wParam, 
                         LONG_PTR lParam);



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


template <typename F>
unsigned create_thread(F f)
{
    thread_data_base* data = NULL;
    try
    {
        data = new thread_data<F>(f);
    }
    catch (std::bad_alloc& )
    {
    	assert(false);
        return 0;
    }
    start_thread(data);
    return data->thread_id_;
}


