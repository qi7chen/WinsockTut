/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */
 
#include "IOCPChatServer.h"
#include <stdio.h>
#include <assert.h>
#include <process.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include "common/Utils.h"

#define ADDR_LEN (sizeof(struct sockaddr_in) + 16)

typedef struct connection_s
{
    WSAOVERLAPPED   overlap;
    SOCKET          socket;
    WSABUF          buf;
    enum OperType   op;
    char            data[1];
}connection_t;

/* parse new accepted socket info */
typedef union accept_info_s
{
    struct  
    {
        connection_t* handle;
        struct sockaddr_in local_addr;
        char reserve1[16];
        struct sockaddr_in remote_addr;
        char reserve2[16];
    }addr;
    char buffer[DEFAULT_BUFFER_SIZE];
}accept_info_t;

/* internal server data structrue */
typedef struct echo_server_s
{
    connection_t*   acceptor;           /* acceptor socket handle */
    HANDLE          completion_port;    /* completion port handle */
}echo_server_t;

static echo_server_t     g_server; /* global echo server object */

/* function pointers */
static LPFN_ACCEPTEX               fnAcceptEx;
static LPFN_GETACCEPTEXSOCKADDRS   fnGetAcceptExSockaddrs;
static LPFN_DISCONNECTEX           fnDisconnectEx;

static int InitExtendFunctionPointer(SOCKET sockfd)
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
        return -1;
    }

    /* GetAcceptExSockaddrs */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_getacceptexaddr,
        sizeof(guid_getacceptexaddr), &fnGetAcceptExSockaddrs, 
        sizeof(fnGetAcceptExSockaddrs), &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() get GetAcceptExSockaddrs pointer failed, %s.\n"), LAST_ERROR_MSG);
        return -1;
    }

    /* DisconnectEx */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_disconnectex,
        sizeof(guid_disconnectex), &fnDisconnectEx, sizeof(fnDisconnectEx),
        &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() get DisconnectEx pointer failed, %s.\n"), LAST_ERROR_MSG);
        return -1;
    }

    return 0;
}

/* free allocated socket resource */
static void FreeConnection(connection_t* conn)
{
    assert(conn);
    closesocket(conn->socket);
    free(conn);
}


/* allocate socket file descriptor and associated handle data */
static connection_t* AllocConnection(void)
{
    connection_t* conn;
    SOCKET sockfd;
    HANDLE handle;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return NULL;
    }
    conn = (connection_t*)malloc(sizeof(connection_t) + DEFAULT_BUFFER_SIZE);
    assert(g_server.completion_port != NULL);
    handle = CreateIoCompletionPort((HANDLE)sockfd, g_server.completion_port, (ULONG_PTR)conn, 0);
    if (handle != g_server.completion_port)
    {
        FreeConnection(conn);
        return NULL;
    }
    memset(conn, 0, sizeof(*conn));
    conn->socket = sockfd;
    conn->op = OperClose;
    conn->buf.buf = conn->data;
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    return conn;
}

/* create a listen socket for accept */
static connection_t* CreateAcceptor(const char* host, int port)
{
    int error;
    SOCKET sockfd;
    struct sockaddr_in addr;
    connection_t* conn;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    conn = AllocConnection();
    if (conn == NULL)
    {
        return NULL;
    }
    sockfd = conn->socket;
    error = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        FreeConnection(conn);
        return NULL;
    }
    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        FreeConnection(conn);
        return NULL;
    }
    
    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now());
    return conn;
}

static int PostAccept(void)
{
    int error;
    SOCKET sockfd;
    accept_info_t* info;
    connection_t* server = g_server.acceptor;
    connection_t* client;

    server->op = OperAccept;
    client = AllocConnection();
    if (client == NULL)
    {
        return -1;
    }

    sockfd = client->socket;
    info = (accept_info_t*)server->data;
    info->addr.handle = client;

    error = fnAcceptEx(server->socket, sockfd, info->buffer + sizeof(client),
         0, ADDR_LEN, ADDR_LEN, NULL, &server->overlap);
    if (error == 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "AcceptEx() failed[%d], %s.\n", error, LAST_ERROR_MSG);
            FreeConnection(server);
            return -1;
        }
    }
    return 0;
}

static void OnClosed(connection_t* conn)
{
    assert(conn);
    fprintf(stdout, ("socket %d closed at %s.\n"), conn->socket, Now());
    FreeConnection(conn);
}

static void OnDisconnect(connection_t* conn)
{
    assert(conn);
    OnClosed(conn);
}

static void OnSend(connection_t* conn)
{
    int error;
    assert(conn);
    conn->op = OperDisconnect;
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    error = fnDisconnectEx(conn->socket, &conn->overlap, 0, 0);
    if (error != 0)
    {
        OnClosed(conn);
    }
}

static void OnRecv(connection_t* conn)
{
    int error;
    DWORD bytes;

    assert(conn);
    bytes = (DWORD)conn->overlap.InternalHigh;
    conn->buf.len = bytes;
    conn->op = OperSend;
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    error = WSASend(conn->socket, &conn->buf, 1, &bytes,
        0, &conn->overlap, NULL);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("WSASend() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            OnClosed(conn);
        }
    }
}

static void OnAccept(connection_t* server)
{
    accept_info_t* info;
    connection_t* client;
    DWORD bytes = 0;
    DWORD flag = 0;
    int error;

    /* parse accepted socket */
    struct sockaddr_in* remote_addr = NULL;
    struct sockaddr_in* local_addr = NULL;
    int remote_len = sizeof(struct sockaddr_in);
    int local_len = sizeof(struct sockaddr_in);

    assert(server);

    info = (accept_info_t*)server->data;
    client = info->addr.handle;
    
    fnGetAcceptExSockaddrs(info->buffer + sizeof(client), 0, ADDR_LEN, ADDR_LEN,
        (struct sockaddr**)&local_addr, &local_len, (struct sockaddr**)&remote_addr, &remote_len);

    client->op = OperRecv;
    error = WSARecv(client->socket, &client->buf, 1, &bytes, &flag,
        &client->overlap, NULL);
    if (error != 0)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stdout, ("WSASend() failed, %d: %s.\n"), error, LAST_ERROR_MSG);
            OnClosed(client);
        }
    }
    fprintf(stdout, ("socket %d accepted at %s.\n"), client->socket, Now());
    PostAccept();
}

static void OnSocketEvent(connection_t* handle)
{
    switch (handle->op)
    {
    case OperAccept:
        OnAccept(handle);
        break;

    case OperRecv:
        OnRecv(handle);
        break;

    case OperSend:
        OnSend(handle);
        break;

    case OperDisconnect:
        OnDisconnect(handle);
        break;

    case OperClose:
        OnClosed(handle);
        break;
    }
}

static int RunEventLoop(connection_t** result)
{
    int error;
    DWORD bytes = 0;
    WSAOVERLAPPED* overlap = NULL;
    connection_t* handle = NULL;
    HANDLE completion_port = g_server.completion_port;

    error = GetQueuedCompletionStatus(completion_port, &bytes,
        (ULONG_PTR*)&handle, &overlap, 50);
    if (error == 0)
    {
        error = WSAGetLastError();
        if (error == WAIT_TIMEOUT)
        {
            return 0;
        }
        if (overlap != NULL)
        {
            handle->op = OperClose;
        }
        if (error == ERROR_INVALID_HANDLE)
        {
            return -1;
        }
    }
    if (handle->op == OperRecv && bytes == 0)
    {
        handle->op = OperDisconnect;
    }
    *result = handle;
    return 0;
}

static int InitServer(const char* host, int port)
{
    connection_t* acceptor;
    HANDLE handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (handle == NULL)
    {
        return -1;
    }
    g_server.completion_port = handle;
    acceptor = CreateAcceptor(host, port);
    if (acceptor == NULL)
    {
        CloseHandle(handle);
        return -2;
    }
    g_server.acceptor = acceptor;
    if (InitExtendFunctionPointer(acceptor->socket) != 0)
    {
        CloseHandle(handle);
        FreeConnection(acceptor);
        return -3;
    }
    return PostAccept();
}

//int StartEchoServer(const char* host, const char* port)
//{
//    if (InitServer(host, atoi(port)) != 0)
//    {
//        return -1;
//    }
//    
//    for (;;)
//    {
//        connection_t* handle = NULL;
//        int r = RunEventLoop(&handle);
//        if (r != 0)
//        {
//            break;
//        }
//        if (handle)
//        {
//            OnSocketEvent(handle);
//        }
//    }
//
//    FreeConnection(g_server.acceptor);
//    CloseHandle(g_server.completion_port);
//    return 0;
//}
