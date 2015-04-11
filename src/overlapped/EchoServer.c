/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "EchoServer.h"
#include <assert.h>
#include <stdio.h>
#include <WinSock2.h>
#include "common/utility.h"

#define LOOP_INTERVAL_MS    50

typedef struct connection_s
{
    WSAOVERLAPPED   overlap;
    SOCKET          socket;
    WSABUF          buf;
    char            data[1];
}connection_t;

static int              g_count = 0;
static connection_t*    g_connections[WSA_MAXIMUM_WAIT_EVENTS];

// allocate data for a socket connection
static int AllocConnetion(SOCKET sockfd)
{
    int idx;
    connection_t* conn = NULL;
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return -1;
    }

    conn = malloc(sizeof(connection_t) + DEFAULT_BUFFER_SIZE);
    memset(conn, 0, sizeof(*conn));
    conn->socket = sockfd;
    conn->buf.buf = conn->data;
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    conn->overlap.hEvent = hEvent;
    idx = g_count++;
    g_connections[idx] = conn;
    return idx;
}

// free allocated data
static void FreeConnection(connection_t* conn)
{
    assert(conn);
    WSACloseEvent(conn->overlap.hEvent);
    closesocket(conn->socket);
    free(conn);
}

static void OnClose(connection_t* conn, int idx)
{
    int i;
    fprintf(stdout, ("socket %d closed at %s.\n"), conn->socket, Now());
    FreeConnection(conn);
    g_connections[idx] = NULL;
    for (i = idx; i < g_count - 1; i++)
    {
        g_connections[i] = g_connections[i + 1];
    }
    g_count--;
}

static void PostRecvRequest(connection_t* conn, int idx)
{
    int error;
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    assert(conn);
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    error = WSARecv(conn->socket, &conn->buf, 1, &dwReadBytes,
        &dwFlag, &conn->overlap, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        OnClose(conn, idx);
    }
}

static int OnAccept(SOCKET sockfd)
{
    int idx;
    if (g_count == WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "MAXIMUM_WAIT_EVENTS(%d) limit.\n", WSA_MAXIMUM_WAIT_EVENTS);
        return -1;
    }
    idx = AllocConnetion(sockfd);
    if (idx < 0)
    {
        return -2;
    }
    PostRecvRequest(g_connections[idx], idx);
    fprintf(stdout, "socket %d connected at %s.\n", sockfd, Now());
    return 0;
}


static void OnRead(connection_t* conn, int idx)
{
    int error;
    DWORD dwBytes = (DWORD)conn->overlap.InternalHigh;
    if (dwBytes == 0)
    {
        OnClose(conn, idx);
        return ;
    }
    conn->buf.len = dwBytes;
    error = WSASend(conn->socket, &conn->buf, 1, &dwBytes,
        0, &conn->overlap, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        OnClose(conn, idx);
        return ;
    }
    shutdown(conn->socket, SD_BOTH);
    PostRecvRequest(conn, idx);
}

static int RunOverlapLoop()
{
    int idx;
    BOOL status;
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    connection_t* conn;
    WSAEVENT hEvent;
    WSAEVENT eventlist[WSA_MAXIMUM_WAIT_EVENTS];

    if (g_count == 0)
    {
        Sleep(LOOP_INTERVAL_MS);
        return 0;
    }

    for (idx = 0; idx < g_count; idx++)
    {
        eventlist[idx] = g_connections[idx]->overlap.hEvent;
    }
    idx = WSAWaitForMultipleEvents(g_count, eventlist, FALSE, LOOP_INTERVAL_MS, FALSE);
    if (idx == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return -1;
    }
    else if (idx == WSA_WAIT_TIMEOUT)
    {
        /* timed out here */
    }
    else if (idx >= WSA_WAIT_EVENT_0 && idx < g_count)
    {
        conn = g_connections[idx];
        if (conn == NULL)
        {
            fprintf(stderr, "event object [%p]not found.\n", &hEvent);
            return 1;
        }
        WSAResetEvent(conn->overlap.hEvent);
        status = WSAGetOverlappedResult(conn->socket, &conn->overlap,
            &dwReadBytes, FALSE, &dwFlag);
        if (status)
        {
            OnRead(conn, idx);
        }
        else
        {
            OnClose(conn, idx);
        }
    }
    return 0;
}

/* create a listen socket for accept */
static SOCKET CreateAcceptor(const char* host, int port)
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
        return 0;
    }
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
    /* set to non-blocking mode */
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }
    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now());
    return sockfd;
}

int StartEchoServer(const char* host, const char* port)
{
    SOCKET acceptor = CreateAcceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return -1;
    }
    for (;;)
    {
        SOCKET sockfd = accept(acceptor, 0, 0);
        if (sockfd != INVALID_SOCKET)
        {
            if (OnAccept(sockfd) != 0)
            {
                closesocket(sockfd);
            }
        }
        else
        {
            if (GetLastError() == WSAEWOULDBLOCK)
            {
                if (RunOverlapLoop() == 0)
                {
                    continue;
                }
            }
            fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
            break;
        }
    }
    return 0;
}