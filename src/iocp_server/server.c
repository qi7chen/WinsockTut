/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */
 
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include <process.h>
#include <MSWSock.h>
#include "common/avl.h"
#include "common/utility.h"

#define ADDR_LEN (sizeof(struct sockaddr_in) + 16)

/* parse new accepted socket info */
typedef union accept_info
{
    struct  
    {
        SOCKET sockfd;
        struct sockaddr_in local_addr;
        char reserve1[16];
        struct sockaddr_in remote_addr;
        char reserve2[16];
    }info;
    char buffer[kDefaultBufferSize];
}accept_info;

/* internal server data structrue */
typedef struct iocp_server
{
    SOCKET      acceptor;           /* acceptor socket handle */
    HANDLE      completion_port;    /* completion port handle */
    avl_tree_t* data_map;           /* total socket handles */
}iocp_server;


static iocp_server     echo_server; /* global echo server object */

/* function pointers */
static LPFN_ACCEPTEX               fnAcceptEx;
static LPFN_GETACCEPTEXSOCKADDRS   fnGetAcceptExSockaddrs;
static LPFN_DISCONNECTEX           fnDisconnectEx;

static int init_extend_function_poiner(SOCKET sockfd)
{
    int error;
    DWORD bytes;
    GUID guid_acceptex = WSAID_ACCEPTEX;
    GUID guid_getacceptexaddr = WSAID_GETACCEPTEXSOCKADDRS;
    GUID guid_disconnectex = WSAID_DISCONNECTEX;

    /* AcceptEx */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_acceptex,
        sizeof(guid_acceptex), &fnAcceptEx, sizeof(fnAcceptEx),
        &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() get AcceptEx pointer failed, %s.\n"), LAST_ERROR_MSG);
        return 0;
    }

    /* GetAcceptExSockaddrs */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_getacceptexaddr,
        sizeof(guid_getacceptexaddr), &fnGetAcceptExSockaddrs, 
        sizeof(fnGetAcceptExSockaddrs), &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() get GetAcceptExSockaddrs pointer failed, %s.\n"), LAST_ERROR_MSG);
        return 0;
    }

    /* DisconnectEx */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_disconnectex,
        sizeof(guid_disconnectex), &fnDisconnectEx, sizeof(fnDisconnectEx),
        &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() get DisconnectEx pointer failed, %s.\n"), LAST_ERROR_MSG);
        return 0;
    }

    return 1;
}

/* free allocated socket resource */
static void free_socket_data(PER_HANDLE_DATA* handle_data)
{
    SOCKET sockfd;
    assert(handle_data);
    sockfd = handle_data->socket;
    avl_delete(echo_server.data_map, (avl_key_t)sockfd);
    closesocket(sockfd);
    free(handle_data);
}


/* allocate socket file descriptor and associated handle data */
static PER_HANDLE_DATA* alloc_socket_data()
{
    PER_HANDLE_DATA* handle_data;
    SOCKET sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return NULL;
    }
    handle_data = (PER_HANDLE_DATA*)malloc(sizeof(PER_HANDLE_DATA));
    if (handle_data == NULL)
    {
        closesocket(sockfd);
        return NULL;
    }
    if (!AssociateDevice(echo_server.completion_port, (HANDLE)sockfd, (ULONG_PTR)handle_data))
    {
        free_socket_data(handle_data);
        return NULL;
    }
    memset(handle_data, 0, sizeof(*handle_data));
    handle_data->socket = sockfd;
    handle_data->opertype = OperClose; 
    handle_data->wsbuf.buf = handle_data->buffer;
    handle_data->wsbuf.len = kDefaultBufferSize;
    
    avl_insert(echo_server.data_map, (avl_key_t)sockfd, handle_data);

    return handle_data;
}


/* create a listen socket for accept */
static SOCKET create_acceptor(const char* host, short port)
{
    int error;
    int sockfd;
    struct sockaddr_in addr;
    PER_HANDLE_DATA* handle_data;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);

    handle_data = alloc_socket_data();
    if (handle_data == NULL)
    {
        return INVALID_SOCKET;
    }
    sockfd = handle_data->socket;
    error = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
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
    
    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now());
    return sockfd;
}

static int post_an_accept()
{
    int error;
    SOCKET acceptor;
    SOCKET sockfd;
    accept_info* info;
    PER_HANDLE_DATA* handle_data;
    PER_HANDLE_DATA* accept_data;

    acceptor = echo_server.acceptor;
    accept_data = avl_find(echo_server.data_map, acceptor);
    if (accept_data == NULL)
    {
        return 0;
    }
    accept_data->opertype = OperAccept;
    handle_data = alloc_socket_data();
    if (handle_data == NULL)
    {
        return 0;
    }

    sockfd = handle_data->socket;
    info = (accept_info*)accept_data->buffer;
    info->info.sockfd = sockfd;

    error = fnAcceptEx(acceptor, sockfd, info->buffer+sizeof(sockfd),
         0, ADDR_LEN, ADDR_LEN, NULL, &handle_data->overlap);
    if (error == 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "AcceptEx() failed[%d], %s.\n", error, LAST_ERROR_MSG);
            free_socket_data(accept_data);
            return 0;
        }
    }
    return 1;
}


static void  on_closed(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    fprintf(stdout, ("socket %d closed at %s.\n"), handle_data->socket, Now());
    free_socket_data(handle_data);
}

static void on_disconnect(PER_HANDLE_DATA* handle_data)
{
    int error;
    assert(handle_data);
    handle_data->opertype = OperClose;
    error = fnDisconnectEx(handle_data->socket, &handle_data->overlap, TF_REUSE_SOCKET, 0);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("DisconnectEx() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            on_closed(handle_data);
        }
    }
}

static void  on_sent(PER_HANDLE_DATA* handle_data)
{
    int error;
    DWORD bytes = 0;
    DWORD flag = 0;

    assert(handle_data);
    handle_data->opertype = OperRecv;
    handle_data->wsbuf.len = sizeof(handle_data->buffer);
    memset(&handle_data->overlap, 0, sizeof(handle_data->overlap));
    error = WSARecv(handle_data->socket, &handle_data->wsbuf, 1, &bytes,
        &flag, &handle_data->overlap, NULL);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("WSARecv() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            on_closed(handle_data);
        }
    }
}

static void on_recv(PER_HANDLE_DATA* handle_data)
{
    int error;
    DWORD bytes;

    assert(handle_data);
    bytes = handle_data->overlap.InternalHigh;
    handle_data->wsbuf.len = bytes;
    handle_data->opertype = OperSend;
    memset(&handle_data->overlap, 0, sizeof(handle_data->overlap));
    error = WSASend(handle_data->socket, &handle_data->wsbuf, 1, &bytes,
        0, &handle_data->overlap, NULL);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("WSASend() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            on_closed(handle_data);
        }
    }
}


static void on_accepted(PER_HANDLE_DATA* listen_handle)
{
    accept_info* info;
    SOCKET sockfd;
    PER_HANDLE_DATA* handle;
    DWORD bytes = 0;
    DWORD flag = 0;
    int error;

    /* parse accepted socket */
    struct sockaddr_in* remote_addr = NULL;
    struct sockaddr_in* local_addr = NULL;
    int remote_len = sizeof(struct sockaddr_in);
    int local_len = sizeof(struct sockaddr_in);

    assert(listen_handle);

    info = (accept_info*)listen_handle->buffer;
    sockfd = info->info.sockfd;
    
    fnGetAcceptExSockaddrs(info->buffer+sizeof(sockfd), 0, ADDR_LEN, ADDR_LEN, 
        (struct sockaddr**)&local_addr, &local_len, (struct sockaddr**)&remote_addr, &remote_len);

    handle = avl_find(echo_server.data_map, (avl_key_t)sockfd);
    if (handle == NULL)
    {
        fprintf(stderr, ("accepted socket %d not found in map.\n"), sockfd);
        return ;
    }

    handle->opertype = OperRecv;
    error = WSARecv(handle->socket, &handle->wsbuf, 1, &bytes, &flag, 
        &handle->overlap, NULL);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("WSASend() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            on_closed(handle);
        }
    }
    fprintf(stdout,("socket %d accepted at %s.\n"), handle->socket, Now());
    post_an_accept();
}


static int event_loop(PER_HANDLE_DATA** handle)
{
    int error;
    DWORD bytes = 0;
    WSAOVERLAPPED* overlap = NULL;
    PER_HANDLE_DATA* handle_data = NULL;
    HANDLE completion_port = echo_server.completion_port;

    error = GetQueuedCompletionStatus(completion_port, &bytes,
        (ULONG_PTR*)&handle_data, &overlap, 500);
    if (error == 0)
    {
        error = WSAGetLastError();
        if (error == WAIT_TIMEOUT)
        {
            return 1;
        }
        if (overlap != NULL)
        {
            handle_data->opertype = OperClose;
        }
        if (error == ERROR_INVALID_HANDLE)
        {
            return 0;
        }
    }
    if (handle_data->opertype == OperRecv && bytes == 0)
    {
        handle_data->opertype = OperDisconnect;
    }
    *handle = handle_data;
    return 1;
}

int server_run()
{
    PER_HANDLE_DATA* handle = NULL;
    if (!event_loop(&handle))
    {
        return 0;
    }
    if (handle == NULL)
    {
        return 1;
    }
    switch(handle->opertype)
    {
    case OperAccept:
        on_accepted(handle);
        break;

    case OperRecv:
        on_recv(handle);
        break;

    case OperSend:
        on_sent(handle);
        break;

    case OperDisconnect:
        on_disconnect(handle);
        break;

    case OperClose:
        on_closed(handle);
        break;
    }
    return 1;
}

int server_init(const char* host, short port)
{
    WSADATA data;
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);

    echo_server.data_map = avl_create_tree();
    if (echo_server.data_map == NULL)
    {
        return 0;
    }
    echo_server.completion_port = CreateCompletionPort(0);
    if (echo_server.completion_port == NULL)
    {
        avl_destroy_tree(echo_server.data_map);
        return 0;
    }
    echo_server.acceptor = create_acceptor(host, port);
    if (echo_server.acceptor == INVALID_SOCKET)
    {
        avl_destroy_tree(echo_server.data_map);
        CloseHandle(echo_server.completion_port);
        return 0;
    }
    if (!init_extend_function_poiner(echo_server.acceptor))
    {
        avl_destroy_tree(echo_server.data_map);
        closesocket(echo_server.acceptor);
        CloseHandle(echo_server.completion_port);
        return 0;
    }

    post_an_accept();
    return 1;
}

void server_destroy()
{
    CloseHandle(echo_server.completion_port);
    avl_destroy_tree(echo_server.data_map);
    WSACleanup();
}
