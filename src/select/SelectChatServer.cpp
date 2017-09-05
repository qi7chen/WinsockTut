/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "SelectChatServer.h"
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "common/Utils.h"


SelectChatServer::SelectChatServer()
{
    acceptor_ = INVALID_SOCKET;
}

SelectChatServer::~SelectChatServer()
{
    closesocket(acceptor_);
    acceptor_ = INVALID_SOCKET;
}

bool SelectChatServer::Init(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port, true);
    if (fd == SOCKET_ERROR)
    {
        return false;
    }
    acceptor_ = fd;
    fprintf(stdout, "server start listen at %s:%s\n", host, port);
    return true;
}

int SelectChatServer::Run()
{
    while (true)
    {
        if (!Poll())
        {
            return 1;
        }
    }
    return 0;
}

bool SelectChatServer::Poll()
{
    struct timeval timeout = {};
    timeout.tv_usec = 200 * 1000; // 20 ms timeout
    fd_set readset = {};
    FD_SET(acceptor_, &readset);
    std::map<SOCKET, int>::iterator iter = connections_.begin();
    for (; iter != connections_.end(); iter++)
    {
        FD_SET(iter->first, &readset);
    }

    int nready = select(0, &readset, NULL, NULL, &timeout);
    if (nready == SOCKET_ERROR)
    {
        fprintf(stderr, ("select(): %s\n"), LAST_ERROR_MSG);
        return false;
    }
    if (nready == 0) // timed out
    {
        return true;
    }
    if (FD_ISSET(acceptor_, &readset))
    {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SOCKET fd = accept(acceptor_, (sockaddr*)&addr, &len);
        if (fd == SOCKET_ERROR)
        {
            fprintf(stderr, "accept(): %s.\n", LAST_ERROR_MSG);
            return false;
        }
        char addrbuf[64] = {};
        inet_ntop(addr.sin_family, &addr.sin_addr, addrbuf, 64);
        fprintf(stdout, "socket %d from %s connected\n", fd, addrbuf);
        // the evil fd size limit of select
        if (connections_.size() >= FD_SETSIZE) 
        {
            fprintf(stderr, "socket %d been kicked due to server connection is full\n", fd);
            closesocket(fd);
        }
        OnNewConnection(fd);
    }
    
    return true;
}

void SelectChatServer::OnNewConnection(SOCKET fd)
{
    // set to non-blocking mode
    unsigned long nonblock = 1;
    if (ioctlsocket(fd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket(): %s\n"), LAST_ERROR_MSG);
        closesocket(fd);
    }
    connections_[fd] = 0;
}
