/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "overlap.h"
#include <assert.h>
#include <stdio.h>
#include "common/utility.h"
#include "common/avl.h"


static avl_tree_t*  g_event_map;        /* event list */
static avl_tree_t*  g_connections_map;  /* socket list */


/* find socket data by its event handle */
static PER_HANDLE_DATA* find_handle_data(WSAEVENT hEvent)
{
    SOCKET sockfd = (SOCKET)avl_find(g_event_map, (avl_key_t)hEvent);
    if (sockfd == 0)
    {
        return NULL;
    }
    return avl_find(g_connections_map, (avl_key_t)sockfd);
}


// allocate data for a socket connection
static PER_HANDLE_DATA* alloc_data(SOCKET sockfd)
{
    PER_HANDLE_DATA* data = NULL;
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return NULL;
    }

    data = (PER_HANDLE_DATA*)malloc(sizeof(PER_HANDLE_DATA));
    data->socket = sockfd;
    data->opertype = OperClose;
    data->wsbuf.buf = data->buffer;
    data->wsbuf.len = sizeof(data->buffer);
    data->overlap.hEvent = hEvent;
    return data;
}

// free allocated data
static void free_data(PER_HANDLE_DATA* data)
{
    assert(data);
    WSACloseEvent(data->overlap.hEvent);
    closesocket(data->socket);
    free(data);
}

static void on_close(PER_HANDLE_DATA* handle)
{
    SOCKET sockfd = handle->socket;
    WSAEVENT hEvent = handle->overlap.hEvent;

    avl_delete(g_event_map, (avl_key_t)hEvent);
    avl_delete(g_connections_map, (avl_key_t)sockfd);
    free_data(handle);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
}

static void post_recv_request(PER_HANDLE_DATA* handle)
{
    int error;
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    assert(handle);
    handle->wsbuf.len = sizeof(handle->buffer);
    error = WSARecv(handle->socket, &handle->wsbuf, 1, &dwReadBytes,
        &dwFlag, &handle->overlap, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle);
    }
}

int on_accept(SOCKET sockfd)
{
    WSAEVENT hEvent;
    PER_HANDLE_DATA* handle;
    size_t count = avl_size(g_event_map);
    if (count == WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "MAXIMUM_WAIT_EVENTS(%d) limit.\n", WSA_MAXIMUM_WAIT_EVENTS);
        return 0;
    }
    handle = alloc_data(sockfd);
    if (handle == NULL)
    {
        return 0;
    }

    hEvent = handle->overlap.hEvent;
    avl_insert(g_event_map, (avl_key_t)hEvent, (void*)sockfd);
    avl_insert(g_connections_map, (avl_key_t)sockfd, handle);

    post_recv_request(handle);
    fprintf(stdout, "socket %d connected at %s.\n", sockfd, Now());
    return 1;
}


static void on_read(PER_HANDLE_DATA* handle)
{
    int error;
    DWORD dwBytes = (DWORD)handle->overlap.InternalHigh;
    if (dwBytes == 0)
    {
        on_close(handle);
        return ;
    }
    handle->wsbuf.len = dwBytes;
    error = WSASend(handle->socket, &handle->wsbuf, 1, &dwBytes,
        0, &handle->overlap, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle);
        return ;
    }
    post_recv_request(handle);
}

int overlap_loop()
{
    int index;
    BOOL status;
    DWORD dwFlag = 0;
    DWORD dwReadBytes = 0;
    PER_HANDLE_DATA* handle;
    WSAEVENT hEvent;
    WSAEVENT eventlist[WSA_MAXIMUM_WAIT_EVENTS];
    int count;

    if (avl_size(g_event_map) == 0)
    {
        Sleep(50);
        return 1;
    }

    count = avl_serialize(g_event_map, (avl_key_t*)eventlist, WSA_MAXIMUM_WAIT_EVENTS);
    index = WSAWaitForMultipleEvents(count, eventlist, FALSE, 50, FALSE);
    if (index == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
    else if (index == WSA_WAIT_TIMEOUT)
    {
        /* timed out here */
    }
    else if (index >= WSA_WAIT_EVENT_0 && index < count)
    {
        hEvent = eventlist[index];
        WSAResetEvent(hEvent);
        handle = find_handle_data(hEvent);
        if (handle == NULL)
        {
            fprintf(stderr, "event object [%p]not found.\n", &hEvent);
            return 1;
        }
        status = WSAGetOverlappedResult(handle->socket, &handle->overlap,
            &dwReadBytes, FALSE, &dwFlag);
        if (status)
        {
            on_read(handle);
        }
        else
        {
            on_close(handle);
        }
    }
    return 1;
}

/* start event loop */
int event_loop(SOCKET acceptor)
{
    SOCKET sockfd = accept(acceptor, 0, 0);
    if (sockfd != INVALID_SOCKET)
    {
        if (!on_accept(sockfd))
        {
            closesocket(sockfd);
        }
        return 1;
    }
    else
    {
        if (GetLastError() == WSAEWOULDBLOCK)
        {
            if (overlap_loop())
            {
                return 1;
            }
        }
        fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
}


/* create a listen socket for accept */
SOCKET create_acceptor(const char* host, int port)
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

int overlap_init()
{
    WSADATA data;
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    g_connections_map = avl_create_tree();
    g_event_map = avl_create_tree();
    CHECK(g_event_map && g_connections_map);
    return 1;
}

void overlap_release()
{
    avl_destroy_tree(g_connections_map);
    avl_destroy_tree(g_event_map);
    WSACleanup();
}
