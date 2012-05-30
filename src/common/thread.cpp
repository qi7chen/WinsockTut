
#include "thread.h"
#include <process.h>



thread_data_base::thread_data_base()        
    : thread_handle_(INVALID_HANDLE_VALUE),
      thread_id_(0)
{
}

thread_data_base::~thread_data_base()
{
    thread_handle_ = INVALID_HANDLE_VALUE;
    thread_id_ = 0;
}



static unsigned CALLBACK run_thread_func(void* pv)
{
    std::auto_ptr<thread_data_base> data((thread_data_base*)pv);
    if (data.get())
    {
        try
        {
            data->run();
            return 0;
        }
        catch (...)
        {
            LOG_ERROR(_T("thread %d encountered unhandled exception!"), data->thread_id_);
            return 1;
        }                
    }

    return 1;
}

bool start_thread(thread_data_base* data)
{
    assert(data);
    uintptr_t thread_handle = _beginthreadex(0, 0, run_thread_func, data, 
        CREATE_SUSPENDED, &data->thread_id_);
    if (thread_handle == 0)
    {
        LOG_ERROR(_T("_beginthreadex() failed"));
        return false;
    }
    else
    {
        data->thread_handle_ = (HANDLE)thread_handle;
        ResumeThread(data->thread_handle_);
    }
    return true;
}
