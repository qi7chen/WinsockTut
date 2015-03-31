/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "select.h"
#include <stdio.h>
#include <WS2tcpip.h>
#include "common/utility.h"
#include "common/avl.h"


#pragma warning(disable:4127)


static avl_tree_t*  g_total_connections;    /* totocal client connections */
static fd_set       g_readset;  /* total socket file descriptor */


static int on_recv(SOCKET sockfd)
{
    char buf[kDefaultBufferSize];
    int bytes = recv(sockfd, buf, kDefaultBufferSize, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, ("recv() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
    if (bytes == 0)
    {
        return 0;
    }

    bytes = send(sockfd, buf, bytes, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, ("send() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    return 1;
}


static int on_accept(SOCKET acceptor)
{
    ULONG nonblock = 1;
    SOCKET sockfd;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);

    /* the evil 64 limit */
    if (avl_size(g_total_connections) == FD_SETSIZE - 1)
    {
        fprintf(stderr, ("reach fd_set size limit(%d).\n"), FD_SETSIZE);
        return 0;
    }
    sockfd = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
    /* set to non-blocking mode */
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return 0;
    }
    avl_insert(g_total_connections, (avl_key_t)sockfd, NULL);
    fprintf(stdout, ("socket %d accepted at %s.\n"), sockfd, Now());
    return 1;
}

static void on_close(SOCKET sockfd)
{
    closesocket(sockfd);
    avl_delete(g_total_connections, (avl_key_t)sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
}

static void on_socket_event(SOCKET acceptor)
{
    int i;
    int array[FD_SETSIZE];
    size_t size = avl_size(g_total_connections);
    avl_serialize(g_total_connections, (avl_key_t*)array, size);

    /* check connection for read/write */
    for (i = 0; i < size; ++i)
    {
        SOCKET sockfd = array[i];
        if (FD_ISSET(sockfd, &g_readset))
        {
            if (!on_recv(sockfd))
            {
                on_close(sockfd);
            }
        }
    }

    /* have new connection? */
    if (FD_ISSET(acceptor, &g_readset))
    {
        on_accept(acceptor);
    }
}

int select_loop(SOCKET acceptor)
{
    int nready;
    int count = 0;
    struct timeval timeout = {0, 500*1000}; /* 50 ms timeout */

    FD_ZERO(&g_readset);
    count = avl_serialize(g_total_connections, (avl_key_t*)g_readset.fd_array, FD_SETSIZE);
    g_readset.fd_count = count;
    if (count < FD_SETSIZE-1) // max monitor number
    {
        FD_SET(acceptor, &g_readset);
    }

    nready = select(0, &g_readset, NULL, NULL, &timeout);
    if (nready == SOCKET_ERROR)
    {
        fprintf(stderr, ("select() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }
    if (nready == 0) /* timed out */
    {
        return 1;
    }

    /* check connection for read/write */
    on_socket_event(acceptor);

    return 1;
}


/* create acceptor socket */
SOCKET  create_acceptor(const char* host, const char* port)
{
    int error;
    ULONG nonblock = 1;
    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo* aiList = NULL;
    struct addrinfo hints;
    struct addrinfo* pinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo() failed, %s:%s, %s.\n", host, port, gai_strerror(error));
        return INVALID_SOCKET;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            fprintf(stderr,"socket() failed, %s", LAST_ERROR_MSG);
            continue;
        }
        error = bind(sockfd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, "bind() failed, addr: %s, len: %d, %s",
                pinfo->ai_addr, pinfo->ai_addrlen, LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        error = listen(sockfd, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, "listen() failed, %s", LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        /* set to non-blocking mode */
        if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
        {
            fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(aiList);
    fprintf(stdout, "server listen at %s:%s\n", host, port);
    return sockfd;
}

int select_init()
{
    WSADATA data;
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    g_total_connections = avl_create_tree();
    CHECK(g_total_connections != NULL);
    return 1;
}

void select_release()
{
    avl_destroy_tree(g_total_connections);
    WSACleanup();
}
