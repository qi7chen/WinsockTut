#include "complete_routine.h"
#include <assert.h>
#include <map>



namespace {

    std::map<int, socket_data*>      g_sockList;
}


// callback routines
static void CALLBACK recv_complete(DWORD error, 
                                   DWORD bytes_transferred, 
                                   WSAOVERLAPPED* overlap, 
                                   DWORD flags);
static void CALLBACK send_complete(DWORD error, 
                                   DWORD bytes_transferred, 
                                   WSAOVERLAPPED* overlap, 
                                   DWORD flags);

socket_data* alloc_data(SOCKET sockfd)
{
    socket_data* data = NULL;
    try
    {
        data = new socket_data();
    }
    catch (std::bad_alloc&)
    {
        fprintf(stderr, ("allocate socket data failed.\n"));
        return NULL;
    }
        
    // Winsock didn't use 'hEvent` in complete routine
    data->overlap_.hEvent = (WSAEVENT)data; 
    data->socket_ = sockfd;
    data->wsabuf_.buf = data->databuf_;
    data->wsabuf_.len = sizeof(data->databuf_);
    
    g_sockList[sockfd] = data;

    return data;
}


void free_data(socket_data* data)
{
    if (data)
    {
        fprintf(stdout, ("socket %d closed at %s.\n"), data->socket_, Now().data());
        closesocket(data->socket_);
        g_sockList.erase(data->socket_);
        delete data;
    }
}

// start read
bool post_recv_request(socket_data* data)
{
    assert(data);
    DWORD flags = 0;
    DWORD recv_bytes = 0;
    int error = WSARecv(data->socket_, &data->wsabuf_, 1, &recv_bytes, &flags, 
        &data->overlap_, recv_complete);
    if (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, ("WSARecv() failed [%d], %s"), data->socket_, LAST_ERROR_MSG);
        free_data(data);
        return false;
    }
    return true;
}



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

    // send back
    memset(&data->overlap_, 0, sizeof(data->overlap_));
    data->overlap_.hEvent = (WSAEVENT)data;
    data->wsabuf_.len = bytes_transferred;
    DWORD bytes_send = 0;
    error = WSASend(data->socket_, &data->wsabuf_, 1, &bytes_send, flags, 
                &data->overlap_, send_complete);
    if (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        free_data(data);
    }
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

    data->wsabuf_.len = sizeof(data->databuf_);
    post_recv_request(data);
}
