/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <WinSock2.h>
#include "common/utility.h"


static int      g_count = 0;
static WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];       /* total events */
static SOCKET   g_connections[WSA_MAXIMUM_WAIT_EVENTS];  /* total socket descriptors */

static int OnClose(SOCKET sockfd, int index, int error)
{
    int i;
    WSAEVENT hEvent = g_events[index];
    if (hEvent == WSA_INVALID_EVENT || hEvent == NULL)
    {
        return -1;
    }
    WSAEventSelect(sockfd, NULL, 0);
    WSACloseEvent(hEvent);
    closesocket(sockfd);
    g_events[index] = WSA_INVALID_EVENT;
    g_connections[index] = INVALID_SOCKET;
    for (i = index; i < g_count-1; ++i)
    {
        g_events[i] = g_events[i + 1];
        g_connections[i] = g_connections[i + 1];
    }
    g_count--;
    fprintf(stderr, ("socket %d closed at %s.\n"), sockfd, Now());
    return 0;
}

static int OnRecv(SOCKET sockfd, int index, int error)
{
    char buffer[DEFAULT_BUFFER_SIZE];
    int bytes = recv(sockfd, buffer, DEFAULT_BUFFER_SIZE, 0);
    if (bytes == SOCKET_ERROR)
    {
        return OnClose(sockfd, index, 0);
    }
    if (bytes == 0)
    {
        return OnClose(sockfd, index, 0);
    }
    /* send back */
    bytes = send(sockfd, buffer, bytes, 0);
    if (bytes == 0)
    {
        return OnClose(sockfd, index, 0);
    }
    shutdown(sockfd, SD_BOTH);
    return 0;
}

static int OnWrite(SOCKET sockfd, int index, int error)
{
    return 0;
}

/* on new connection arrival */
static int OnAccept(SOCKET sockfd)
{
    WSAEVENT hEvent;
    if (g_count == WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "maximum event size limit(%d).\n", WSA_MAXIMUM_WAIT_EVENTS);
        return -1;
    }
    hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return -2;
    }
    /* associate event handle */
    if (WSAEventSelect(sockfd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        WSACloseEvent(hEvent);
        fprintf(stderr, ("WSAEventSelect() failed, %s"), LAST_ERROR_MSG);
        return -3;
    }

    g_events[g_count] = hEvent;
    g_connections[g_count] = sockfd;
    g_count++;

    fprintf(stdout, ("socket %d connected at %s.\n"), sockfd, Now());
    return 0;
}


static int HandleEvents(SOCKET sockfd, int index, const WSANETWORKEVENTS* events_struct)
{
    const int* errorlist = events_struct->iErrorCode;
    int events = events_struct->lNetworkEvents;
    if (events & FD_READ)
    {
        OnRecv(sockfd, index, errorlist[FD_READ_BIT]);
    }
    if (events & FD_WRITE)
    {
        OnWrite(sockfd, index, errorlist[FD_WRITE_BIT]);
    }
    if (events & FD_CLOSE)
    {
        OnClose(sockfd, index, errorlist[FD_CLOSE_BIT]);
    }
    return 0;
}

static int AsynSelectLoop(void)
{
    size_t index;
    int nready;
    WSAEVENT hEvent;
    SOCKET sockfd;
    WSANETWORKEVENTS event_struct;

    if (g_count == 0)
    {
        Sleep(10);
        return 0;
    }
    nready = WSAWaitForMultipleEvents(g_count, g_events, FALSE, 100, FALSE);
    if (nready == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return -1;
    }
    else if (nready == WSA_WAIT_TIMEOUT)
    {
    }
    else if (nready == WSA_WAIT_IO_COMPLETION)
    {
    }
    else
    {
        index = WSA_WAIT_EVENT_0 + nready;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            fprintf(stderr, "invalid event index: %d.\n", index);
            return -2;
        }
        sockfd = g_connections[index];
        hEvent = g_events[index];
        if (WSAEnumNetworkEvents(sockfd, hEvent, &event_struct) == SOCKET_ERROR)
        {
            fprintf(stderr, ("WSAEnumNetworkEvents() failed, %s"), LAST_ERROR_MSG);
            OnClose(sockfd, index, 0);
            return -3;
        }
        HandleEvents(sockfd, index, &event_struct);
    }
    return 0;
}

/* create acceptor */
static SOCKET CreateAcceptor(const char* host, int port)
{
    int error;
    ULONG nonblock = 1;
    SOCKET acceptor;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);
    acceptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (acceptor == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }
    error = bind(acceptor, (struct sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return INVALID_SOCKET;
    }
    if (listen(acceptor, SOMAXCONN) == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return INVALID_SOCKET;
    }
    /* set to non-blocking mode */
    if (ioctlsocket(acceptor, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return INVALID_SOCKET;
    }
    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now());
    return acceptor;
}

int StartEchoServer(const char* host, const char* port)
{
    SOCKET sockfd;
    SOCKET acceptor = CreateAcceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return -1;
    }
    for (;;)
    {
        SOCKET sockfd = accept(acceptor, NULL, NULL);
        if (sockfd != SOCKET_ERROR)
        {
            if (OnAccept(sockfd) != 0)
                break;
        }
        else
        {
            if (AsynSelectLoop() != 0)
                break;
        }
    }

    return 0;
}