// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoClient.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <functional>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "SocketOpts.h"

using namespace std::placeholders;

EchoClient::EchoClient(PollerBase* poller)
    :fd_(INVALID_SOCKET)
{
    poller_ = poller;
}

EchoClient::~EchoClient()
{
    Cleanup();
}

void EchoClient::Cleanup()
{
    fprintf(stderr, "%d closed\n", fd_);
    poller_->RemoveFd(fd_);
    closesocket(fd_);
    fd_ = INVALID_SOCKET;
}

void EchoClient::Start(const char* host, const char* port)
{
    SOCKET fd = Connect(host, port);
    CHECK(fd != INVALID_SOCKET);
    fd_ = fd;
    poller_->AddFd(fd, this);
    poller_->SetPollIn(fd);
    poller_->SetPollOut(fd);
}

void EchoClient::OnReadable()
{
    char buf[1024] = {};
    int nbytes = ReadSome(fd_, buf, 1024);
    if (nbytes < 1)
    {
        Cleanup();
    }
    else
    {
        fprintf(stdout, "%d recv %d bytes\n", fd_, nbytes);
    }
}

void EchoClient::OnWritable()
{
    const char msg[] = "a quick brown fox jumps over the lazy dog";
    int nbytes = WriteSome(fd_, msg, (int)strlen(msg));
    if (nbytes < 0)
    {
        Cleanup();
    }
    else
    {
        fprintf(stdout, "%d send %d bytes\n", fd_, nbytes);
    }
}

SOCKET EchoClient::Connect(const char* host, const char* port)
{
    SOCKET fd = INVALID_SOCKET;
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;        // both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host, port, &hints, &aiList);
    if (err != 0)
    {
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return INVALID_SOCKET;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
            continue;
        }
        // set to non-blocking mode
        if (SetNonblock(fd, true) == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("ioctlsocket(): %s\n", LAST_ERROR_MSG);
            closesocket(fd);
            fd = INVALID_SOCKET;
            continue;
        }
        err = connect(fd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
        if (err == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << StringPrintf("connect(): %s\n", LAST_ERROR_MSG);
                closesocket(fd);
                fd = INVALID_SOCKET;
            }
        }
        break; // succeed
    }
    freeaddrinfo(aiList);
    return fd;
}


