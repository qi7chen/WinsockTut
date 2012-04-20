

#include "complete_routine.h"
#include <map>


static std::map<int, socket_data*> g_debug;



socket_data* alloc_data(SOCKET sockfd)
{
    socket_data* data = NULL;
    try
    {
        data = new socket_data();
    }
    catch (std::bad_alloc&)
    {
        LOG_DEBUG(_T("allocate socket data failed"));
        return NULL;
    }
    
    data->socket_ = sockfd;
    data->overlap_.hEvent = (WSAEVENT)data; // windows didn't use hEvent in complete routine
    data->wsabuf_.buf = data->databuf_;
    data->wsabuf_.len = sizeof(data->databuf_);
    
    g_debug[sockfd] = data;

    return data;
}


void free_data(socket_data*& data)
{
    if (data)
    {
        g_debug.erase(data->socket_);
        closesocket(data->socket_);
        delete data;        
        data = NULL;
    }
}


bool post_recv_request(socket_data* data)
{
    assert(data);
    DWORD flags = 0;
    DWORD recv_bytes = 0;
    int error = WSARecv(data->socket_, &data->wsabuf_, 1, &recv_bytes, &flags, 
        &data->overlap_, recv_complete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        LOG_DEBUG(_T("WSARecv() failed"));
        free_data(data);
        return false;
    }
    if (error == 0 && recv_bytes == 0) // client closed
    {
        free_data(data);
        return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////

void CALLBACK recv_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap, 
                            DWORD flags)
{
    assert(overlap);
    socket_data* data = (socket_data*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }

    // send data back
    data->wsabuf_.len = bytes_transferred;
    DWORD bytes_send = 0;
    error = WSASend(data->socket_, &data->wsabuf_, 1, &bytes_send, flags, 
                &data->overlap_, send_complete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        LOG_DEBUG(_T("WSASend() failed"));
        free_data(data);
        return ;
    }
    if (error == 0 && bytes_send == 0) // client closed
    {
        free_data(data);
        return ;
    }

    // post another recv
    data->wsabuf_.len = sizeof(data->databuf_);
    post_recv_request(data);
}


void CALLBACK send_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap,
                            DWORD /*flags*/)
{
    assert(overlap);
    socket_data* data = (socket_data*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }
}