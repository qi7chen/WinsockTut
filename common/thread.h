/**
 *  @file:   thread.h
 *  @brief:  a simple thread class
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once

#include "utility.h"
#include <memory>



class thread
{
public:
    struct thread_data_base;

    template <typename F>
    class thread_data;

public:
    thread()
    {}

    template <typename F>
    thread(F f)
    {
        thread_data_base* pbase = nullptr;
        try
        {
            pbase = new thread_data<F>(f);
        }
        catch (std::bad_alloc&)
        {
            return ;
        }
        
        thread_info_.reset(pbase);
        start_thread();
    }

    
    void    terminate(unsigned exit_code);
    void    join(unsigned milsec);

private:
    thread(const thread&);
    thread& operator = (const thread&);

    void    start_thread();

    static unsigned CALLBACK run_thread_func(void* pv);

private:
    std::shared_ptr<thread_data_base>   thread_info_;
};



struct thread::thread_data_base
{
    HANDLE      thread_handle_;
    unsigned    thread_id_;

    thread_data_base()
        : thread_handle_(INVALID_HANDLE_VALUE),
        thread_id_(0)
    {}

    virtual ~thread_data_base() 
    {
        thread_handle_ = INVALID_HANDLE_VALUE;
        thread_id_ = 0;
    }

    virtual void run() = 0;    
};


template <typename F>
class thread::thread_data 
    : public thread::thread_data_base
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
