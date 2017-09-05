/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */


#include "Utils.h"
#include <time.h>
#include <stdio.h>
#include <WS2tcpip.h>

#define THREAD_LOCAL    __declspec(thread)

const char*  Now()
{
    static THREAD_LOCAL char buffer[MAX_PATH];
    struct tm st;
    time_t now = time(NULL);
    localtime_s(&st, &now);
    strftime(buffer, _countof(buffer), ("%Y-%m-%d %H:%M:%S"), &st);
    return buffer;
}

const char* GetErrorMessage(DWORD dwErrorCode)
{
    static THREAD_LOCAL char description[MAX_PATH];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, description, MAX_PATH-1, NULL);
    return description;
}


// create acceptor socket
SOCKET CreateTCPAcceptor(const char* host, const char* port, bool nonblock)
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
        fprintf(stderr, "getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return INVALID_SOCKET;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            fprintf(stderr, "socket(): %s\n", LAST_ERROR_MSG);
            continue;
        }
        err = bind(sockfd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (err == SOCKET_ERROR)
        {
            fprintf(stderr, "%s bind(): %s\n", pinfo->ai_addr, LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        err = listen(sockfd, SOMAXCONN);
        if (err == SOCKET_ERROR)
        {
            fprintf(stderr, "listen(): %s\n", LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        if (nonblock)
        {
            // set to non-blocking mode
            unsigned long value = 1;
            if (ioctlsocket(sockfd, FIONBIO, &value) == SOCKET_ERROR)
            {
                fprintf(stderr, ("ioctlsocket(): %s\n"), LAST_ERROR_MSG);
                closesocket(sockfd);
                sockfd = INVALID_SOCKET;
                continue;
            }
        }
        break;
    }
    freeaddrinfo(aiList);
    return sockfd;
}
