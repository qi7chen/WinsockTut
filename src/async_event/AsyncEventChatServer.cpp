/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "AsyncEventChatServer.h"
#include <assert.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include "common/Utils.h"


AsyncEventChatServer::AsyncEventChatServer()
{
}

AsyncEventChatServer::~AsyncEventChatServer()
{
    closesocket(acceptor_);
    acceptor_ = INVALID_SOCKET;
}

bool AsyncEventChatServer::Init(const char* host, const char* port)
{
    SOCKET fd = CreateTCPAcceptor(host, port, true);
    if (fd == INVALID_SOCKET)
    {
        return false;
    }
    acceptor_ = fd;
    return true;
}

int AsyncEventChatServer::Run()
{
    for (;;)
    {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SOCKET fd = accept(acceptor_, (sockaddr*)&addr, &len);
        if (fd == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                fprintf(stderr, "accept(): %s", LAST_ERROR_MSG);
                return 1;
            }
            if (connections_.empty())
            {
                Sleep(50); // sleep for a while
                continue;
            }
            int r = Poll();
            if (r != 0)
            {
                return r;
            }
        }
        else
        {
            char addrbuf[64] = {};
            inet_ntop(addr.sin_family, &addr.sin_addr, addrbuf, 64);
            fprintf(stdout, "socket %d from %s connected\n", fd, addrbuf);
            HandleAccept(fd);
        }
    }
    return 0;
}

int AsyncEventChatServer::Poll()
{
    std::vector<WSAEVENT> events;
    events.reserve(connections_.size());
    std::map<SOCKET, WSAEVENT>::iterator iter = connections_.begin();
    for (; iter != connections_.end(); ++iter)
    {
        events.push_back(iter->second);
    }
    int nready = WSAWaitForMultipleEvents(events.size(), &events[0], FALSE, 100, FALSE);
    if (nready == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents(): %s"), LAST_ERROR_MSG);
        return 1;
    }
    else if (nready == WSA_WAIT_TIMEOUT)
    {
    }
    else
    {
        WSANETWORKEVENTS event_struct = {};
        int index = WSA_WAIT_EVENT_0 + nready;
        assert(index < WSA_MAXIMUM_WAIT_EVENTS);
        WSAEVENT hEvent = events[index];
        SOCKET fd = events_[hEvent];
        if (WSAEnumNetworkEvents(fd, hEvent, &event_struct) == SOCKET_ERROR)
        {
            fprintf(stderr, ("WSAEnumNetworkEvents(): %s"), LAST_ERROR_MSG);
            return 1;
        }
        HandleEvents(fd, &event_struct);
    }
    return 0;
}

void AsyncEventChatServer::HandleAccept(SOCKET fd)
{
    if (connections_.size() >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        fprintf(stderr, "socket %d been kicked due to server connection is full\n", fd);
        closesocket(fd);
        return;
    }
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, "WSACreateEvent(): %s\n", LAST_ERROR_MSG);
        closesocket(fd);
        return;
    }
    // associate event handle
    if (WSAEventSelect(fd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAEventSelect(): %s"), LAST_ERROR_MSG);
        WSACloseEvent(hEvent);
        closesocket(fd);
        return;
    }
    connections_[fd] = hEvent;
    events_[hEvent] = fd;
}

void AsyncEventChatServer::HandleEvents(SOCKET fd, WSANETWORKEVENTS* event)
{
    const int* errorlist = event->iErrorCode;
    int events = event->lNetworkEvents;
    if (events & FD_READ)
    {
        //OnRecv(sockfd, index, errorlist[FD_READ_BIT]);
    }
    if (events & FD_WRITE)
    {
        //OnWrite(sockfd, index, errorlist[FD_WRITE_BIT]);
    }
    if (events & FD_CLOSE)
    {
        //OnClose(sockfd, index, errorlist[FD_CLOSE_BIT]);
    }
}
