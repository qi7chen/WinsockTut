/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "EchoServer.h"
#include <stdio.h>
#include <WS2tcpip.h>
#include "common/utility.h"


#pragma warning(disable:4127)

static SOCKET   g_connections[FD_SETSIZE];  /* total socket file descriptor */
static int      g_count = 0;

static int OnRecv(SOCKET sockfd)
{
    char buf[DEFAULT_BUFFER_SIZE];
    int bytes = recv(sockfd, buf, DEFAULT_BUFFER_SIZE, 0);
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
    shutdown(sockfd, SD_BOTH);
    return 1;
}

static int OnAccept(SOCKET acceptor)
{
    ULONG nonblock = 1;
    SOCKET sockfd;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);

    /* the evil 64 limit */
    if (g_count == FD_SETSIZE - 1)
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
    g_connections[g_count++] = sockfd;
    fprintf(stdout, ("socket %d accepted at %s.\n"), sockfd, Now());
    return 1;
}

static void OnClose(SOCKET sockfd)
{
    int i;
    for (i = 0; i < g_count; ++i)
    {
        if (g_connections[i] == sockfd)
        {
            g_connections[i] = INVALID_SOCKET;
            break;
        }
    }
    while (i < g_count - 1)
    {
        g_connections[i] = g_connections[i + 1];
    }
    g_count--;
    closesocket(sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
}

static void OnSocketEvent(SOCKET acceptor, fd_set* readset)
{
    int i;

    /* check connection for read/write */
    for (i = 0; i < g_count; ++i)
    {
        SOCKET sockfd = g_connections[i];
        if (FD_ISSET(sockfd, readset))
        {
            if (!OnRecv(sockfd))
            {
                OnClose(sockfd);
            }
        }
    }

    /* have new connection? */
    if (FD_ISSET(acceptor, readset))
    {
        OnAccept(acceptor);
    }
}

static int SelectLoop(SOCKET acceptor)
{
    int nready;
    fd_set readset;
    struct timeval timeout = {0, 100*1000}; /* 10 ms timeout */

    memcpy(readset.fd_array, g_connections, g_count * sizeof(SOCKET));
    readset.fd_count = g_count;
    FD_SET(acceptor, &readset);

    nready = select(0, &readset, NULL, NULL, &timeout);
    if (nready == SOCKET_ERROR)
    {
        fprintf(stderr, ("select() failed, %s"), LAST_ERROR_MSG);
        return -1;
    }
    if (nready == 0) /* timed out */
    {
        return 0;
    }

    /* check connection for read/write */
    OnSocketEvent(acceptor, &readset);

    return 0;
}

/* create acceptor socket */
static SOCKET  CreateAcceptor(const char* host, const char* port)
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

int StartEchoServer(const char* host, const char* port)
{
    SOCKET acceptor = CreateAcceptor(host, port);
    if (acceptor == INVALID_SOCKET)
    {
        return -1;
    }
    while (SelectLoop(acceptor) == 0)
    {
    }
    closesocket(acceptor);
    return 0;
}