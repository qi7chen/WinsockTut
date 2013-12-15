#include "overlap.h"
#include <assert.h>
#include <map>



struct socket_manager
{
    std::map<SOCKET, WSAEVENT>  event_list;
    std::map<WSAEVENT, PER_HANDLE_DATA*>  socket_list;
};

static socket_manager     g_socket_mgr;


// 根据事件对象查找套接字数据
PER_HANDLE_DATA* find_handle_data(WSAEVENT hEvent)
{
    std::map<WSAEVENT, PER_HANDLE_DATA*>::const_iterator iter = g_socket_mgr.socket_list.find(hEvent);
    if (iter == g_socket_mgr.socket_list.end())
    {
        return NULL;
    }
    return iter->second;
}

int make_event_array(WSAEVENT* array, int max_count)
{
    int count = 0;
    for (std::map<SOCKET, WSAEVENT>::const_iterator iter = g_socket_mgr.event_list.begin();
        iter != g_socket_mgr.event_list.end() && max_count-- > 0; ++iter)
    {
        array[count++] = iter->second;
    }
    return count;
}

PER_HANDLE_DATA* alloc_data(SOCKET sockfd)
{
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
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
    closesocket(data->socket_);
    delete data;
}


void on_close(PER_HANDLE_DATA* handle)
{
    assert(handle);
    fprintf(stdout, ("socket %d closed at %s.\n"), handle->socket_, Now().data());    
    SOCKET sockfd = handle->socket_;
    WSAEVENT hEvent = handle->overlap_.hEvent;
    g_socket_mgr.event_list.erase(sockfd);
    g_socket_mgr.socket_list.erase(hEvent);
    free_data(handle);    
}

void post_recv_request(PER_HANDLE_DATA* handle)
{
    assert(handle);
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    handle->wsbuf_.len = sizeof(handle->buffer_);
    int error = ::WSARecv(handle->socket_, &handle->wsbuf_, 1, &dwReadBytes, 
        &dwFlag, &handle->overlap_, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle);
    }
}

bool on_accept(SOCKET sockfd)
{
    int count = g_socket_mgr.event_list.size();
    if (count == WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "Got 64 limit.\n");
        return false;
    }
    PER_HANDLE_DATA* handle = alloc_data(sockfd);
    if (handle == NULL)
    {
        return false;
    }
    WSAEVENT hEvent = handle->overlap_.hEvent;
    g_socket_mgr.event_list[sockfd] = hEvent;
    g_socket_mgr.socket_list[hEvent] = handle;

    post_recv_request(handle);
    fprintf(stdout, "socket %d connected at %s.\n", sockfd, Now().data());
    return true;
}


// 处理读取完成
void on_read(PER_HANDLE_DATA* handle)
{
    DWORD dwReadBytes = handle->overlap_.InternalHigh;
    if (dwReadBytes == 0)
    {
        on_close(handle);
        return ;
    }

    handle->wsbuf_.len = dwReadBytes;
    DWORD dwSentBytes = 0;
    int error = ::WSASend(handle->socket_, &handle->wsbuf_, 1, &dwSentBytes,
        0, &handle->overlap_, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle);
        return ;
    }
    // 发起I/O读取请求
    post_recv_request(handle);
}


// 事件循环
bool event_loop()
{        
    if (g_socket_mgr.event_list.empty())
    {
        ::Sleep(100);
        return true;
    }

    WSAEVENT eventlist[WSA_MAXIMUM_WAIT_EVENTS] = {};
    int count = make_event_array(eventlist, WSA_MAXIMUM_WAIT_EVENTS);
    int index = ::WSAWaitForMultipleEvents(count, eventlist, FALSE, 50, FALSE);
    if (index == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    else if (index == WSA_WAIT_TIMEOUT)
    {
    }
    else if (index >= WSA_WAIT_EVENT_0 && index < count)
    {
        WSAEVENT hEvent = eventlist[index];
        ::WSAResetEvent(hEvent);

        DWORD dwFlag = 0;
        DWORD dwReadBytes = 0;
        PER_HANDLE_DATA* handle = find_handle_data(hEvent);
        if (handle == NULL)
        {
            fprintf(stderr, "event object [%p]not found.\n", &hEvent);
            return true;
        }
        BOOL status = ::WSAGetOverlappedResult(handle->socket_, &handle->overlap_, &dwReadBytes,
            FALSE, &dwFlag);
        if (status)
        {
            on_read(handle);
        }
        else
        {
            on_close(handle);
        }
    }
    return true;
}

