#include "complete_routine.h"
#include <assert.h>
#include <map>



namespace {

// total client connections
static std::map<int, socket_data*>      g_sockList;
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

// allocate data for socket connection
socket_data* alloc_data(SOCKET sockfd)
{
    socket_data* data = new socket_data();
        
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


// create acceptor
SOCKET  create_listen_socket(const char* host, int port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() [%s:%d]failed, %s"), host, port, LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    // set to non-blocking mode
    ULONG nonblock = 1;
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    fprintf(stderr, ("server start listen [%s:%d] at %s.\n"), host, port, Now().data());
    return sockfd;
}
