// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "ChatClient.h"
#include <WS2tcpip.h>
#include <functional>
#include "Common/Logging.h"
#include "Common/StringUtil.h"


using namespace std::placeholders;

ChatClient::ChatClient(IOServiceBase* service)
    : fd_(INVALID_SOCKET), service_(service)
{
}

ChatClient::~ChatClient()
{

}

int ChatClient::Start(const char* host, const char* port)
{
    return Connect(host, port);
}

int ChatClient::Connect(const char* host, const char* port)
{
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host, port, &hints, &aiList);
    if (err != 0)
    {
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return -1;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        SOCKET fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (fd == INVALID_SOCKET)
        {
            continue;
        }
        int r = service_->AsyncConnect(fd, pinfo, std::bind(&ChatClient::OnConnect, this, _1));
        if (r < 0)
        {
            closesocket(fd);
            continue;
        }
        fd_ = fd;
        break;
    }
    freeaddrinfo(aiList);
    return 0;
}

void ChatClient::OnConnect(int error)
{
    if (error == 0)
    {
        
    }
}
