/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "SocketChatServer.h"
#include <stdio.h>
#include <assert.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include "common/Utils.h"

struct ArgList
{
    SocketChatServer* server;
    SOCKET fd;
};

/* thread for each connection */
static unsigned CALLBACK CientThread(void* param)
{
    SOCKET sockfd = (SOCKET)param;
    char databuf[DEFAULT_BUFFER_SIZE];
    int bytes = recv(sockfd, databuf, DEFAULT_BUFFER_SIZE, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, "socket %d recv() failed, %s", sockfd, LAST_ERROR_MSG);
        goto cleanup;
    }
    else if (bytes == 0) /* closed */
    {
        goto cleanup;
    }
    /* send back */
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, "socket %d send() failed, %s", sockfd, LAST_ERROR_MSG);
        goto cleanup;
    }
cleanup:
    closesocket(sockfd);
    fprintf(stdout, "socket %d closed at %s.\n", sockfd, Now());
    return 0;
}


SocketChatServer::SocketChatServer()
{
    acceptor_ = INVALID_SOCKET;
}

SocketChatServer::~SocketChatServer()
{

}

bool SocketChatServer::Init(const char* host, const char* port)
{
    SOCKET fd = CreateAcceptor(host, port);
    if (fd == SOCKET_ERROR)
    {
        return false;
    }
    acceptor_ = fd;
    fprintf(stdout, "server start listen at %s:%s\n", host, port);
    return true;
}

int SocketChatServer::Run()
{
    struct sockaddr_in addr;
    int len = sizeof(addr);
    for (;;)
    {
        SOCKET fd = accept(acceptor_, (struct sockaddr*)&addr, &len);
        if (fd == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                fprintf(stderr, "accept() failed, %s.\n", LAST_ERROR_MSG);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "socket %d accepted, %s.\n", fd, Now());
            ArgList* args = new ArgList;
            args->server = this;
            args->fd = fd;
            _beginthreadex(NULL, 0, ClientThreadFunc, (void*)args, 0, NULL);
        }

        // sleep for a while
        Sleep(50);
    }
    return 0;
}

// create acceptor socket
SOCKET SocketChatServer::CreateAcceptor(const char* host, const char* port)
{
    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host, port, &hints, &aiList);
    if (err != 0)
    {
        fprintf(stderr, "getaddrinfo() failed, %s:%s, %s.\n", host, port, gai_strerror(err));
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
        err = bind(sockfd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (err == SOCKET_ERROR)
        {
            fprintf(stderr, "bind() failed, addr: %s, len: %d, %s",
                pinfo->ai_addr, pinfo->ai_addrlen, LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        err = listen(sockfd, SOMAXCONN);
        if (err == SOCKET_ERROR)
        {
            fprintf(stderr, "listen() failed, %s", LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        // set to non-blocking mode
        unsigned long nonblock = 1;
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
    return sockfd;
}

unsigned CALLBACK SocketChatServer::ClientThreadFunc(void* args)
{
    return 0;
}
