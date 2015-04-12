/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "EchoClient.h"
#include <winsock2.h>
#include <mswsock.h >
#include <assert.h>
#include <stdio.h>
#include "common/utility.h"


typedef struct connection_s
{
    WSAOVERLAPPED   overlap;
    SOCKET          socket;
    WSABUF          buf;
    enum OperType   op;
    char            data[1];
}connection_t;

typedef struct event_loop_s
{
    LPFN_CONNECTEX      fnConnectEx;
    HANDLE              completion_port;
}event_loop_t;


static event_loop_t  default_loop;    /* global internal loop */


static SOCKET CreateSocket(void)
{
    int error;
    struct sockaddr_in local_addr;
    SOCKET sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    /* ConnectEx() need bind() first */
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    error = bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s."), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }
    return sockfd;
}

/* allocate handle data for socket descriptor */
static connection_t* AllocConnection(SOCKET sockfd)
{
    connection_t* conn = (connection_t*)malloc(sizeof(connection_t) + DEFAULT_BUFFER_SIZE);
    memset(conn, 0, sizeof(connection_t));
    conn->socket = sockfd;
    conn->op = OperConnect;
    conn->buf.buf = conn->data;
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    return conn;
}

/* free handle data */
static void FreeConnection(connection_t* conn)
{
    SOCKET sockfd;
    assert(conn);
    sockfd = conn->socket;
    closesocket(sockfd);
    free(conn);
}

/* connection closed */
static void OnClose(connection_t* conn)
{
    SOCKET sockfd;
    assert(conn);
    sockfd = conn->socket;
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
    FreeConnection(conn);
}

/* send a message after connected */
static void OnConnect(connection_t* conn)
{
    int error;
    DWORD bytes;
    const char* msg = "GET /index.html HTTP/1.0\r\n\r\n";
    size_t len = strlen(msg);

    assert(conn);
    fprintf(stdout, ("socket %d connected at %s.\n"), conn->socket, Now());

    memcpy(conn->data, msg, len);
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    conn->buf.len = (ULONG)len;
    conn->op = OperSend;

    error = WSASend(conn->socket, &conn->buf, 1, &bytes,
        0, &conn->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSASend() failed, socket: %d, %d: %s", conn->socket,
                error, LAST_ERROR_MSG);
            OnClose(conn);
        }
        OnClose(conn);
    }
}

/* send message back after recieved */
static void OnRecv(connection_t* conn)
{
    int error;
    DWORD bytes;

    assert(conn);

    /* pause */
    Sleep(200);

    bytes = (DWORD)conn->overlap.InternalHigh;
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    conn->buf.len = bytes;
    conn->buf.buf[bytes] = '\0';
    conn->op = OperSend;

    error = WSASend(conn->socket, &conn->buf, 1, &bytes,
        0, &conn->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSASend() failed, socket: %d, %d: %s", conn->socket,
                error, LAST_ERROR_MSG);
            OnClose(conn);
        }
    }
}

static void OnSend(connection_t* conn)
{
    int error;
    DWORD bytes = 0;
    DWORD flag = 0;

    assert(conn);
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    conn->op = OperRecv;
    error = WSARecv(conn->socket, &conn->buf, 1, &bytes,
        &flag, &conn->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSARecv() failed, socket: %d, %d: %s", conn->socket,
                error, LAST_ERROR_MSG);
            OnClose(conn);
        }
    }
}

static int CreateConnections(const char* host, short port, int num)
{
    int i;
    int error;
    int count;
    HANDLE handle;
    SOCKET sockfd;
    connection_t* conn;
    HANDLE completion_port;
    struct sockaddr_in remote_addr;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host);
    remote_addr.sin_port = htons(port);

    count = 0;
    completion_port = default_loop.completion_port;
    for (i = 0; i < num; i++)
    {
        sockfd = CreateSocket();
        if (sockfd == INVALID_SOCKET)
        {
            continue;
        }
        conn = AllocConnection(sockfd);
        if (conn == NULL)
        {
            closesocket(sockfd);
            continue;
        }
        handle = CreateIoCompletionPort((HANDLE)sockfd, default_loop.completion_port, (ULONG_PTR)conn, 0);
        if (handle != default_loop.completion_port)
        {
            FreeConnection(conn);
            continue;
        }
        error = default_loop.fnConnectEx(sockfd, (struct sockaddr*)&remote_addr,
            sizeof(remote_addr), NULL, 0, NULL, &conn->overlap);
        if (error == 0 && WSAGetLastError() != WSA_IO_PENDING)
        {
            fprintf(stderr, ("ConnectEx() failed [%d], %s."), sockfd, LAST_ERROR_MSG);
            FreeConnection(conn);
            continue;
        }
        count++;
    }
    return count;
}

int RunEventLoop(int timeout)
{
    int error;
    DWORD bytes = 0;
    connection_t* conn = NULL;
    WSAOVERLAPPED* overlap = NULL;

    error = GetQueuedCompletionStatus(default_loop.completion_port, &bytes,
        (ULONG_PTR*)&conn, &overlap, timeout);
    if (error == 0)
    {
        error = GetLastError();
        if (error == WAIT_TIMEOUT)
        {
            return 0;
        }
        fprintf(stderr, "%d: %s", error, LAST_ERROR_MSG);
        if (overlap != NULL)
        {
            conn->op = OperClose;
        }
        else
        {
            if (error == ERROR_INVALID_HANDLE)
            {
                return -1;
            }
            return 0;
        }
    }

    switch (conn->op)
    {
    case OperConnect:
        OnConnect(conn);
        break;

    case OperRecv:
        OnRecv(conn);
        break;

    case OperSend:
        OnSend(conn);
        break;

    case OperClose:
        OnClose(conn);
        break;

    default:
        assert(0);
    }
    return 0;
}

int InitEventLoop(void)
{
    DWORD bytes = 0;
    int error;
    SOCKET sockfd = INVALID_SOCKET;
    GUID guid_connectex = WSAID_CONNECTEX;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("WSAIoctl() failed, %s."), LAST_ERROR_MSG);
        return -1;
    }

    /* obtain ConnectEx function pointer */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connectex,
        sizeof(guid_connectex), &default_loop.fnConnectEx, sizeof(default_loop.fnConnectEx),
        &bytes, 0, 0);
    closesocket(sockfd);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() failed, %s."), LAST_ERROR_MSG);
        return -1;
    }

    /* create I/O completion port handle */
    default_loop.completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (default_loop.completion_port == NULL)
    {
        fprintf(stderr, ("CreateCompletionPort() failed, %s."), LAST_ERROR_MSG);
        return -1;
    }

    return 0;
}

int StartEchoClient(int count, const char* host, const char* port)
{
    if (InitEventLoop() != 0)
    {
        return -1;
    }
    CreateConnections(host, (short)atoi(port), count);
    for (;;)
    {
        if (RunEventLoop(50) != 0)
            break;
    }
    CloseHandle(default_loop.completion_port);
    return 0;
}