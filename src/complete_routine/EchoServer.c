/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "EchoServer.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <WinSock2.h>
#include "common/utility.h"


typedef struct connection_s
{
    WSAOVERLAPPED   overlap;
    SOCKET          socket;     /* os handle */
    WSABUF          buf;        /* winsock buffer */
    char            data[1];    /* recv buffer */
}connection_t;


/* callback routines */
static void CALLBACK OnRecvComplete(DWORD error,
                                    DWORD bytes_transferred,
                                    WSAOVERLAPPED* overlap,
                                    DWORD flags);

static void CALLBACK OnSendComplete(DWORD error,
                                    DWORD bytes_transferred,
                                    WSAOVERLAPPED* overlap,
                                    DWORD flags);


/* allocate associated data for each socket */
static connection_t* AllocConnection(SOCKET sockfd)
{
    connection_t* conn = malloc(sizeof(connection_t) + DEFAULT_BUFFER_SIZE);
    memset(conn, 0, sizeof(*conn));
    /* winsock didn't use 'hEvent` in complete routine I/O model */
    conn->overlap.hEvent = (WSAEVENT)conn;
    conn->socket = sockfd;
    conn->buf.buf = conn->data;
    conn->buf.len = DEFAULT_BUFFER_SIZE;
    return conn;
}


static void FreeConnection(connection_t* conn)
{
    if (conn)
    {
        SOCKET sockfd = conn->socket;
        closesocket(sockfd);
        free(conn);
        fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
    }
}

/* post an asynchounous recv request */
static int PostRecvRequest(connection_t* conn)
{
    DWORD flags = 0;
    DWORD recv_bytes = 0;
    int error = WSARecv(conn->socket, &conn->buf, 1, &recv_bytes, &flags,
        &conn->overlap, OnRecvComplete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, ("WSARecv() failed [%d], %s"), conn->socket, LAST_ERROR_MSG);
        FreeConnection(conn);
        return -1;
    }
    return 0;
}

void CALLBACK OnRecvComplete(DWORD error,
                             DWORD bytes_transferred,
                             WSAOVERLAPPED* overlap,
                             DWORD flags)
{
    DWORD bytes_send = 0;
    connection_t* conn = (connection_t*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        FreeConnection(conn);
        return ;
    }

    /* send back */
    memset(&conn->overlap, 0, sizeof(conn->overlap));
    conn->overlap.hEvent = (WSAEVENT)conn;
    conn->buf.len = bytes_transferred;
    error = WSASend(conn->socket, &conn->buf, 1, &bytes_send, flags,
        &conn->overlap, OnSendComplete);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        FreeConnection(conn);
    }
}

void CALLBACK OnSendComplete(DWORD error,
                             DWORD bytes_transferred,
                             WSAOVERLAPPED* overlap,
                             DWORD flags)
{
    connection_t* conn = (connection_t*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        FreeConnection(conn);
        return ;
    }
    shutdown(conn->socket, SD_BOTH);
    PostRecvRequest(conn);
}

/* create listen socket for accept */
static SOCKET  CreateAcceptor(const char* host, int port)
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

static int CompletionLoop(SOCKET acceptor)
{
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET socknew = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (socknew != INVALID_SOCKET)
    {
        connection_t* data = AllocConnection(socknew);
        fprintf(stderr, ("socket %d accepted at %s.\n"), socknew, Now());
        if (data)
        {
            PostRecvRequest(data);
        }
    }
    else
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
            return -1;
        }
        else
        {
            SleepEx(50, TRUE); /* make current thread alertable */
        }
    }
    return 0;
}

int StartEchoServer(const char* host, const char* port)
{
    SOCKET acceptor = CreateAcceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return -1;
    }
    while (CompletionLoop(acceptor) == 0)
    {
    }

    return 0;
}