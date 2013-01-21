
#include "overlap_worker.h"
#include <algorithm>

using namespace std;


// pre-declaration
PER_HANDLE_DATA* alloc_data(SOCKET sockfd);
void free_data(PER_HANDLE_DATA* data);
void post_recv_request(PER_HANDLE_DATA* handle, socket_data* dataList);
void on_accepted(SOCKET sockfd, socket_data* dataList);
void on_read(PER_HANDLE_DATA* handle, socket_data* dataList);
void on_close(PER_HANDLE_DATA* handle, socket_data* dataList);



void worker_routine()
{
    // total socket list
    socket_data dataList = {};

    for (;;)
    {
        // handle thread message(s)
        MSG msg = {};
        if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (::GetMessage(&msg, NULL, 0, 0))
            {
                switch(msg.message)
                {
                case WM_NEW_SOCKET:
                    on_accepted(msg.wParam, &dataList);
                    break;
                }
            }
        }
        // no connection yet
        if (dataList.total_count == 0)
        {
            ::Sleep(1);
            continue;
        }

        int index = ::WSAWaitForMultipleEvents(dataList.total_count, dataList.eventList, 
            FALSE, MAX_TIMEOUT, FALSE);
        if (index == WSA_WAIT_TIMEOUT)
        {
            // do timers here
        }
        else if (index == WSA_WAIT_FAILED)
        {
            _tprintf(_T("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        }
        else if (index >= WSA_WAIT_EVENT_0 && index < MAX_COUNT)
        {
            ::WSAResetEvent(dataList.eventList[index]);

            DWORD dwFlag = 0;
            DWORD dwReadBytes = 0;
            PER_HANDLE_DATA* handle = dataList.dataList[index];
            BOOL status = ::WSAGetOverlappedResult(handle->socket_, &handle->overlap_, &dwReadBytes,
                FALSE, &dwFlag);
            if (status)
            {
                on_read(handle, &dataList);
            }
            else
            {
                on_close(handle, &dataList);
            }
        }
    }
}

void post_recv_request(PER_HANDLE_DATA* handle, socket_data* data)
{
    assert(handle && data);
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    handle->wsbuf_.len = sizeof(handle->buffer_);
    int error = ::WSARecv(handle->socket_, &handle->wsbuf_, 1, &dwReadBytes, 
        &dwFlag, &handle->overlap_, NULL);
    if (error == 0 || 
        (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
    {
        // succeed
    }
    else
    {
        on_close(handle, data);
    }
}


void on_accepted(SOCKET sockfd, socket_data* data)
{
    PER_HANDLE_DATA* handle = alloc_data(sockfd);
    if (handle != NULL)
    {
        unsigned pos = data->total_count++;
        data->eventList[pos] = handle->overlap_.hEvent;
        data->dataList[pos] = handle;

        post_recv_request(handle, data);
    }
}

void on_read(PER_HANDLE_DATA* handle, socket_data* data)
{
    DWORD dwReadBytes = handle->overlap_.InternalHigh;
    if (dwReadBytes == 0)
    {
        on_close(handle, data);
        return ;
    }

    handle->wsbuf_.len = dwReadBytes;
    DWORD dwSentBytes = 0;
    int error = ::WSASend(handle->socket_, &handle->wsbuf_, 1, &dwSentBytes,
        0, &handle->overlap_, NULL);
    if (error == 0 || 
        (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
    {
        // succeed
    }
    else
    {
        on_close(handle, data);
    }

    // deliver next request
    post_recv_request(handle, data);
}

void on_close(PER_HANDLE_DATA* handle, socket_data* data)
{
    assert(handle && data);
    _tprintf(_T("socket %d closed at %s.\n"), handle->socket_, Now().data());
    unsigned count = data->total_count--;
    WSAEVENT hEvent = handle->overlap_.hEvent;
    std::remove(data->eventList, data->eventList + count, hEvent);
    std::remove(data->dataList, data->dataList + count, handle);
    closesocket(handle->socket_);
    free_data(handle);    
}

PER_HANDLE_DATA* alloc_data(SOCKET sockfd)
{
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        _tprintf(_T("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return NULL;
    }

    PER_HANDLE_DATA* data = new PER_HANDLE_DATA();
    data->socket_ = sockfd;
    data->opertype_ = OperClose;
    data->wsbuf_.buf = data->buffer_;
    data->wsbuf_.len = sizeof(data->buffer_);
    data->overlap_.hEvent = hEvent;

    return data;
}

void free_data(PER_HANDLE_DATA* data)
{
    assert(data);
    WSACloseEvent(data->overlap_.hEvent);
    delete data;
}
