/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include <stdio.h>
#include <assert.h>
#include "appdef.h"
#include "common/avl.h"
#include "common/utility.h"


static avl_tree_t*  g_total_connections; /* total client connections */


static int OnAccept(HWND hwnd, SOCKET acceptor)
{
    int error;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET sockfd = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("accpet() failed, %s"), LAST_ERROR_MSG);
        return -1;
    }
    error = WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return -2;
    }
    avl_insert(g_total_connections, (avl_key_t)sockfd, NULL);
    fprintf(stdout, ("socket %d accepted at %s.\n"), sockfd, Now());
    return 0;
}

static void OnClose(SOCKET sockfd)
{
    closesocket(sockfd);
    avl_delete(g_total_connections, (avl_key_t)sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
}

static int OnRecv(SOCKET sockfd)
{
    char buffer[DEFAULT_BUFFER_SIZE];
    int bytes = recv(sockfd, buffer, DEFAULT_BUFFER_SIZE, 0);
    if (bytes == SOCKET_ERROR)
    {
        OnClose(sockfd);
        return -1;
    }
    if (bytes == 0)
    {
        OnClose(sockfd);
        return -2;
    }
    bytes = send(sockfd, buffer, bytes, 0);
    if (bytes == 0)
    {
        OnClose(sockfd);
        return -3;
    }
    shutdown(sockfd, SD_BOTH);
    return 0;
}

int InitEchoServer(HWND hwnd, const char* host, const char* port)
{
    int error;    
    SOCKET acceptor;
    struct sockaddr_in addr;

    g_total_connections = avl_create_tree();
    CHECK(g_total_connections != NULL);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)atoi(port));
    acceptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (acceptor == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        avl_destroy_tree(g_total_connections);
        return -1;
    }

    error = bind(acceptor, (struct sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        avl_destroy_tree(g_total_connections);
        return -2;
    }

    error = listen(acceptor, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        avl_destroy_tree(g_total_connections);
        return -3;
    }

    /* http://msdn.microsoft.com/en-us/library/windows/desktop/ms741540(v=vs.85).aspx
     *
     * The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
     * regardless of the value of lEvent.
     */
    if (WSAAsyncSelect(acceptor, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        avl_destroy_tree(g_total_connections);
        return -4;
    }

    avl_insert(g_total_connections, (avl_key_t)acceptor, NULL);
    fprintf(stdout, ("server listen at %s:%s, %s.\n"), host, port, Now());

    return 0;
}

void CloseServer()
{
    size_t i;
    size_t count = avl_size(g_total_connections);
    SOCKET* array = (SOCKET*)malloc(count * sizeof(int));
    avl_serialize(g_total_connections, (avl_key_t*)array, count);
    for (i = 0; i < count; ++i)
    {
        OnClose(array[i]);
    }
    free(array);
    avl_destroy_tree(g_total_connections);
    
    fprintf(stdout, ("server[%d] closed at %s.\n"), count, Now());
}

int HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error)
{
    if (error)
    {
        OnClose(sockfd);
        return 0;
    }
    switch(event)
    {
    case FD_ACCEPT:
        return OnAccept(hwnd, sockfd);

    case FD_READ:
        return OnRecv(sockfd);

    case FD_WRITE:
        /* do nothing */
        break;

    case FD_CLOSE:
        OnClose(sockfd);
        break;

    default:
        assert(!"cannot reach here!");
    }
    return 0;
}