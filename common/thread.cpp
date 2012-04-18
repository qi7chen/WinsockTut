
#include "thread.h"
#include <process.h>



unsigned thread::run_thread_func(void* pv)
{
    thread_data_base* info = (thread_data_base*)pv;
    if (info)
    {
        try
        {
            info->run();
        }
        catch (...)
        {
            LOG_ERROR(_T("thread %d encountered unhandled exception!"), info->thread_id_);
        }                
    }

    return 0;
}

void thread::start_thread()
{
    uintptr_t thread_handle = _beginthreadex(0, 0, run_thread_func, thread_info_.get(), 
        CREATE_SUSPENDED, &thread_info_->thread_id_);
    if (thread_handle == 0)
    {
        LOG_ERROR(_T("_beginthreadex() failed"));
    }
    else
    {
        thread_info_->thread_handle_ = (HANDLE)thread_handle;
        ResumeThread(thread_info_->thread_handle_);
    }
}

void thread::terminate(unsigned exitcode)
{
    if (thread_info_.get())
    {
        ::TerminateThread(thread_info_->thread_handle_, exitcode);
    }
}

void thread::join(unsigned milsec)
{
    if (thread_info_.get())
    {
        ::WaitForSingleObject(thread_info_->thread_handle_, milsec);
    }
}
