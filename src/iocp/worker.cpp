
#include "worker.h"
#include "iocp.h"
#include <assert.h>
#include "../common/utility.h"


static bool worker_loop(iocp_server* server)
{
    DWORD bytes_transferred = 0;
    WSAOVERLAPPED* overlap = NULL;
    PER_HANDLE_DATA* handle_data = NULL;
    BOOL status = ::GetQueuedCompletionStatus(server->completion_port(), &bytes_transferred,
        (ULONG_PTR*)&handle_data, &overlap, 500);
    if (status == FALSE)
    {
        if (overlap != NULL)
        {
            handle_data->opertype_ = OperDisconnect;
        }
        else 
        {
            if (GetLastError() == WAIT_TIMEOUT)
            {
                return true;
            }
            else
            {
                fprintf(stderr, ("GetQueuedCompletionStatus() failed, %s.\n"), LAST_ERROR_MSG);
                return false;
            }
        }
    }

    assert(handle_data);
    if (handle_data->opertype_ == OperRecv && bytes_transferred == 0)
    {
        handle_data->opertype_ = OperDisconnect;
    }

    server->push_command(handle_data);
    return true;
}

unsigned CALLBACK NativeThreadFunc(void* param)
{
    assert(param);
    iocp_server* server = (iocp_server*)param;
    while (worker_loop(server))
    {
    }
    return 0;
}
