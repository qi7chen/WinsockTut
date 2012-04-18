
#include "worker.h"


void worker::wait_loop(overlap_server* server)
{
    for (;;)
    {
        int index = WSAWaitForMultipleEvents(total_count_, event_list_, FALSE, INFINITE, FALSE);
        if (index == WSA_WAIT_FAILED)
        {
            LOG_PRINT(_T("WSAWaitForMultipleEvents() failed"));
            break;
        }

    }
}