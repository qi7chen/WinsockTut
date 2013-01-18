#include "thread.h"
#include "logging.h"
#include <assert.h>
#include <process.h>


namespace detail {

unsigned CALLBACK run_thread_func(void* pv)
{
    assert(pv);
    std::auto_ptr<thread_data_base> sp(reinterpret_cast<thread_data_base*>(pv));

    try
    {
        sp->run();
    }
    catch(...)
    {
        LOG_DEBUG(_T("Unhandled exception occurs of thread %d\n"), sp->thread_id_);
    }

    return 0;
}

bool start_thread(thread_data_base* ptr)
{
    assert(ptr);
    unsigned thread_handle = _beginthreadex(NULL, 0, detail::run_thread_func,
        ptr, CREATE_SUSPENDED, &ptr->thread_id_);
    if (thread_handle == 0)
    {
        LOG_ERROR(_T("_beginthreadex() failed"));
        return false;
    }
    ptr->thread_handle_ = (HANDLE)thread_handle;
    ::ResumeThread(ptr->thread_handle_);
    return true;
}

thread_data_base::thread_data_base()
    : thread_handle_(INVALID_HANDLE_VALUE), thread_id_(0)
{
}

thread_data_base::~thread_data_base()
{
    thread_handle_ = INVALID_HANDLE_VALUE;
    thread_id_ = 0;
}

} // namespace detail



bool send_message_to(unsigned thread_id, 
                     unsigned msg, 
                     unsigned param1 /* = 0 */, 
                     long param2 /* = 0 */)
{
    BOOL status = ::PostThreadMessage(thread_id, msg, param1, param2);
    if (status != TRUE)
    {
        LOG_ERROR(_T("PostThreadMessage() failed"));
        return false;
    }
    return true;
}