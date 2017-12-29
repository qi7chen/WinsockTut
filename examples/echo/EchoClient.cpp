// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoClient.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <functional>
#include "Common/Util.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"

using namespace std::placeholders;

EchoClient::EchoClient(IOMode mode)
{
    loop_ = new EventLoop(mode);
    fd_ = INVALID_SOCKET;
}

EchoClient::~EchoClient()
{
    closesocket(fd_);
    fd_ = INVALID_SOCKET;
    delete loop_;
    loop_ = NULL;
}

void EchoClient::Cleanup()
{
    loop_->DelEvent(fd_, EV_READABLE | EV_WRITABLE);
    closesocket(fd_);
}

void EchoClient::Start(const char* host, const char* port)
{
    bool ok = Connect(host, port);
    CHECK(ok);
    loop_->AddEvent(fd_, EV_WRITABLE, std::bind(&EchoClient::OnWritable, 
        this, _1, _2, _3));
    loop_->AddEvent(fd_, EV_READABLE, std::bind(&EchoClient::OnReadable, 
        this, _1, _2, _3));
}

void EchoClient::OnReadable(SOCKET fd, int mask, int err)
{
    if (err != 0)
    {
        LOG(ERROR) << GetErrorMessage(err);
        Cleanup();
        return ;
    }
    int bytes = 0;
    char buf[1024] = {};
    while (true)
    {
        int r = recv(fd_, buf, sizeof(buf), 0);
        if (r == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << "recv: " << LAST_ERROR_MSG;
                Cleanup();
            }
            break;
        }
        if (r == 0) // EOF
        {
            Cleanup();
            break;
        }
        bytes += r;
    }
}

void EchoClient::OnWritable(SOCKET fd, int mask, int err)
{
    if (err != 0)
    {
        LOG(ERROR) << GetErrorMessage(err);
        Cleanup();
        return ;
    }
    const char msg[] = "a quick brown fox jumps over the lazy dog";
    int r = send(fd_, msg, sizeof(msg), 0);
    if (r == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LOG(ERROR) << "send: " << LAST_ERROR_MSG;
           Cleanup();
        }
    }
}

void EchoClient::Run()
{
    loop_->Run();
}

bool EchoClient::Connect(const char* host, const char* port)
{
    SOCKET fd = INVALID_SOCKET;
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
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return false;
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
        unsigned long value = 1;
        if (ioctlsocket(fd, FIONBIO, &value) == SOCKET_ERROR)
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
                return false;
            }
        }
        break;
    }
    freeaddrinfo(aiList);
    fd_ = fd;
    return fd_ != INVALID_SOCKET;
}


