/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include "common/utility.h"
#include "common/avl.h"


static avl_tree_t*  g_events_map;       /* total events */
static avl_tree_t*  g_connections_map;  /* total socket descriptors */


static int on_close(SOCKET sockfd, int error)
{
    WSAEVENT hEvent = (WSAEVENT)avl_find(g_connections_map, (avl_key_t)sockfd);
    if (hEvent == NULL)
    {
        fprintf(stderr, ("on_close(): socket %d not found.\n"), sockfd);
        return 0;
    }
    WSAEventSelect(sockfd, NULL, 0);
    WSACloseEvent(hEvent);
    closesocket(sockfd);
    avl_delete(g_connections_map, (avl_key_t)sockfd);
    avl_delete(g_events_map, (avl_key_t)hEvent);
    fprintf(stderr, ("socket %d closed at %s.\n"), sockfd, Now());
    return 1;
}


static int on_recv(SOCKET sockfd, int error)
{
    char databuf[kDefaultBufferSize];
    int bytes = recv(sockfd, databuf, kDefaultBufferSize, 0);
    if (bytes == SOCKET_ERROR && bytes == 0)
    {
        return on_close(sockfd, 0);
    }
    /* send back */
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        return on_close(sockfd, 0);
    }
    return 1;
}

static int on_write(SOCKET sockfd, int error)
{
    return 1;
}

/* on new connection arrival */
static int on_accept(SOCKET sockfd)
{
    WSAEVENT hEvent;
    
    if (avl_size(g_events_map) == WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "maximum event size limit(%d).\n", WSA_MAXIMUM_WAIT_EVENTS);
        return 1;
    }
    hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
    /* associate event handle */
    if (WSAEventSelect(sockfd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        WSACloseEvent(hEvent);
        fprintf(stderr, ("WSAEventSelect() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    avl_insert(g_events_map, (avl_key_t)hEvent, (void*)sockfd);
    avl_insert(g_connections_map, (avl_key_t)sockfd, hEvent);

    fprintf(stdout, ("socket %d connected at %s.\n"), sockfd, Now());
    return 1;
}


static int  handle_event(SOCKET sockfd, const WSANETWORKEVENTS* events_struct)
{
    const int* errorlist = events_struct->iErrorCode;
    int events = events_struct->lNetworkEvents;
    if (events & FD_READ)
    {
        on_recv(sockfd, errorlist[FD_READ_BIT]);
    }
    if (events & FD_WRITE)
    {
        on_write(sockfd, errorlist[FD_WRITE_BIT]);
    }
    if (events & FD_CLOSE)
    {
        on_close(sockfd, errorlist[FD_CLOSE_BIT]);
    }
    return 1;
}

int asyn_select_loop()
{
    size_t index;
    int nready;
    int count;
    WSAEVENT hEvent;
    SOCKET sockfd;
    WSANETWORKEVENTS event_struct;
    WSAEVENT eventlist[WSA_MAXIMUM_WAIT_EVENTS];

    if (avl_size(g_events_map) == 0)
    {
        Sleep(10);
        return 1;
    }
    count = avl_serialize(g_events_map, (avl_key_t*)eventlist, WSA_MAXIMUM_WAIT_EVENTS);
    nready = WSAWaitForMultipleEvents(count, eventlist, FALSE, 100, FALSE);
    if (nready == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return 0;
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
            return 1;
        }
        hEvent = eventlist[index];
        sockfd = (SOCKET)avl_find(g_events_map, (avl_key_t)hEvent);
        if (sockfd == 0)
        {
            fprintf(stderr, "invalid event object %p.\n", &hEvent);
            return 1;
        }
        if (WSAEnumNetworkEvents(sockfd, hEvent, &event_struct) == SOCKET_ERROR)
        {
            fprintf(stderr, ("WSAEnumNetworkEvents() failed, %s"), LAST_ERROR_MSG);
            on_close(sockfd, 0);
            return 0;
        }
        handle_event(sockfd, &event_struct);
    }
    return 1;
}

int event_loop(SOCKET acceptor)
{
    SOCKET sockfd = accept(acceptor, NULL, NULL);
    if (sockfd != SOCKET_ERROR)
    {
        if (!on_accept(sockfd))
            return 0;
    }
    else
    {
        if (!asyn_select_loop())
            return 0;
    }
    return 1;
}


/* create acceptor */
SOCKET create_acceptor(const char* host, int port)
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

int async_event_init()
{
    WSADATA data;
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    g_events_map = avl_create_tree();
    g_connections_map = avl_create_tree();
    CHECK(g_events_map && g_connections_map);
    return 1;
}

void async_event_release()
{
    avl_destroy_tree(g_events_map);
    avl_destroy_tree(g_connections_map);
    WSACleanup();
}
