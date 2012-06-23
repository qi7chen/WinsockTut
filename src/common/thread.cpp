
#include "thread.h"
#include <process.h>



static unsigned CALLBACK run_thread_func(void* pv)
{
    std::auto_ptr<thread_data_base> data((thread_data_base*)pv);    
    if (data.get())
    {
        // force this thread to create a message queue
        while (!PostThreadMessage(data->thread_id_, 0, 0, 0))
        {
            Sleep(1);
        }

        try
        {
            data->run();
        }
        catch (...)
        {
            LOG_ERROR(_T("thread %d encountered unhandled exception!"), data->thread_id_);
            return 1;
        }
        return 0;
    }

    return 1;
}


void start_thread(thread_data_base* data)
{
    uintptr_t thread_handle = _beginthreadex(0, 0, run_thread_func, data, 
        CREATE_SUSPENDED, &data->thread_id_);
    if (thread_handle == 0)
    {
        LOG_ERROR(_T("_beginthreadex() failed"));
    }
    else
    {
        data->thread_handle_ = (HANDLE)thread_handle;
        ::ResumeThread(data->thread_handle_);
    }
}

// send message
bool send_message_to(unsigned thread_id, 
                     unsigned msg, 
                     UINT_PTR wParam, 
                     LONG_PTR lParam)
{
    bool bOk = PostThreadMessage(thread_id, msg, wParam, lParam) == TRUE;
    if (!bOk)
    {
        LOG_ERROR(_T("PostThreadMessage() failed"));
    }
    return bOk;
}


//////////////////////////////////////////////////////////////////////////

thread_data_base::thread_data_base()
    : thread_handle_(INVALID_HANDLE_VALUE), thread_id_(0)
{
}

thread_data_base::~thread_data_base()
{
    thread_handle_ = INVALID_HANDLE_VALUE;
    thread_id_ = 0;
}