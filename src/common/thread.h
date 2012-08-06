#pragma once


#include "config.h"
#include <memory>


namespace detail {

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

bool start_thread(thread_data_base* ptr);

} // namespace detail



template <typename F>
unsigned create_thread(F f)
{
    detail::thread_data_base* ptr(new detail::thread_data<F>(f));
    if (detail::start_thread(ptr))
    {
        return ptr->thread_id_;
    }
    return 0;
}

// send message to a thread's queue
bool send_message_to(unsigned thread_id, 
                     unsigned msg, 
                     unsigned param1 = 0, 
                     long param2 = 0);
