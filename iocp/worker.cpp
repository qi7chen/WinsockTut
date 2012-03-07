
#include "worker.h"
#include "iocp.h"
#include <assert.h>
#include "../common/utility.h"





void run_worker_loop(iocp_server* server)
{
    assert(server);
    DWORD bytes_transferred = 0;
    WSAOVERLAPPED* overlap = NULL;
    PER_HANDLE_DATA* handle_data = NULL;

    for (;;)
    {
        BOOL status = ::GetQueuedCompletionStatus(server->completion_port(), &bytes_transferred,
                      (ULONG_PTR*)&handle_data, &overlap, INFINITE);
        if (status == FALSE)
        {
            if (overlap != NULL)
            {
                handle_data->opertype_ = OperDisconnect;
            }
            else
            {
                break;
            }
        }

        assert(handle_data);
        if (handle_data->opertype_ == OperRecv && bytes_transferred == 0)
        {
            handle_data->opertype_ = OperDisconnect;
        }

        server->push_command(handle_data);
    }
}