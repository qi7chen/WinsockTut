#include "complete_routine.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include "common/avl.h"
#include "common/utility.h"


typedef struct socket_data
{
    // socket handle
    SOCKET  socket_;    /* os handle */
    WSABUF  wsabuf_;    /* winsock buffer */
    char    databuf_[kDefaultBufferSize]; /* recv buffer */
    WSAOVERLAPPED   overlap_;
}socket_data_t;


/* total socket connections */
static avl_tree_t*  g_connections_map;

/* callback routines */
static void CALLBACK recv_complete(DWORD error, 
                                   DWORD bytes_transferred, 
                                   WSAOVERLAPPED* overlap, 
                                   DWORD flags);

static void CALLBACK send_complete(DWORD error, 
                                   DWORD bytes_transferred, 
                                   WSAOVERLAPPED* overlap, 
                                   DWORD flags);


/* allocate associated data for each socket */
static socket_data_t* alloc_data(SOCKET sockfd)
{
    socket_data_t* data = malloc(sizeof(socket_data_t));
        
    // winsock didn't use 'hEvent` in complete routine I/O model
    data->overlap_.hEvent = (WSAEVENT)data; 
    data->socket_ = sockfd;
    data->wsabuf_.buf = data->databuf_;
    data->wsabuf_.len = sizeof(data->databuf_);

    avl_insert(g_connections_map, (avl_key_t)sockfd, NULL);
    return data;
}


static void free_data(socket_data_t* data)
{
    if (data)
    {
        SOCKET sockfd = data->socket_;        
        closesocket(sockfd);
        avl_delete(g_connections_map, (avl_key_t)sockfd);
        fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
        free(data);
    }
}

/* post an asynchounous recv request */
static int post_recv_request(socket_data_t* data)
{    
    DWORD flags = 0;
    DWORD recv_bytes = 0;
    int error = WSARecv(data->socket_, &data->wsabuf_, 1, &recv_bytes, &flags, 
        &data->overlap_, recv_complete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, ("WSARecv() failed [%d], %s"), data->socket_, LAST_ERROR_MSG);
        free_data(data);
        return 0;
    }
    return 1;
}



void CALLBACK recv_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap, 
                            DWORD flags)
{
    DWORD bytes_send = 0;
    socket_data_t* data = (socket_data_t*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }

    /* send back */
    memset(&data->overlap_, 0, sizeof(data->overlap_));
    data->overlap_.hEvent = (WSAEVENT)data;
    data->wsabuf_.len = bytes_transferred;    
    error = WSASend(data->socket_, &data->wsabuf_, 1, &bytes_send, flags, 
                &data->overlap_, send_complete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        free_data(data);
    }
}


void CALLBACK send_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap,
                            DWORD flags)
{
    socket_data_t* data = (socket_data_t*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }

    data->wsabuf_.len = sizeof(data->databuf_);
    post_recv_request(data);
}


/* create listen socket for accept */
SOCKET  create_acceptor(const char* host, int port)
{
    int error;
    SOCKET sockfd;
    ULONG nonblock = 1;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }
    error = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
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
    /* set to non-blocking mode */
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    fprintf(stderr, ("server start listen [%s:%d] at %s.\n"), host, port, Now());
    return sockfd;
}

int event_loop(SOCKET acceptor)
{
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET socknew = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (socknew != INVALID_SOCKET)
    {
        socket_data_t* data = alloc_data(socknew);
        fprintf(stderr, ("socket %d accepted at %s.\n"), socknew, Now());
        if (data)
        {
            post_recv_request(data);
        }
    }
    else
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {   
            fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
            return 0;
        }
        else
        {
            SleepEx(50, TRUE); // make thread alertable
        }
    }
    return 1;
}