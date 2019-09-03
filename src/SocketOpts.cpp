// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "SocketOpts.h"
#include <assert.h>
#include <WS2tcpip.h>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringUtil.h"



// set socket to non-blocking mode
int SetNonblock(SOCKET fd, bool nonblock)
{
    unsigned long val = nonblock ? 1 : 0;
    int r = ioctlsocket(fd, FIONBIO, &val);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("ioctlsocket(): %s\n", LAST_ERROR_MSG);
    }
    return r;
}

int BindAnyAddr(SOCKET fd, int family)
{
    sockaddr* paddr = nullptr;
    int addrlen = 0;
    sockaddr_in addr4 = {};
    sockaddr_in6 addr6 = {};
    switch (family)
    {
    case AF_INET:
        addr4.sin_family = family;
        addr4.sin_addr.S_un.S_addr = INADDR_ANY;
        paddr = (sockaddr*)&addr4;
        addrlen = sizeof(addr4);
        break;
    case AF_INET6:
        addr6.sin6_family = family;
        addr6.sin6_addr = in6addr_any;
        paddr = (sockaddr*)&addr6;
        addrlen = sizeof(addr6);
        break;
    default:
        assert(false && "invalid net family");
        abort();
        return -1;
    }
    int r = bind(fd, paddr, addrlen);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << "bind: " << LAST_ERROR_MSG;
        return -1;
    }
    return 0;
}

int ReadSome(SOCKET fd, void* buf, int size)
{
    int nbytes = 0;
    while (true)
    {
        int r = recv(fd, (char*)buf + nbytes, size - nbytes, 0);
        if (r > 0)
        {
            nbytes += r;
            continue;
        }
        else if (r == 0) // EOF
        {
            return -1; 
        }
        else if (r == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << r << LAST_ERROR_MSG;
                return -1;
            }
            break;
        }
        break;
    }
    return nbytes;
}

int WriteSome(SOCKET fd, const void* buf, int size)
{
    int nbytes = 0;
    while (nbytes < size)
    {
        int r = send(fd, (char*)buf + nbytes, size - nbytes, 0);
        if (r >= 0)
        {
            nbytes += r;
            continue;
        }
        else if (r == SOCKET_ERROR)
        {
            r = WSAGetLastError();
            if (r != WSAEWOULDBLOCK)
            {
                 LOG(ERROR) << r << LAST_ERROR_MSG;
                 return -1;
            }
            break;
        }
        break;
    }
    return nbytes;
}

bool IsSelfConnection(SOCKET fd)
{
	sockaddr_storage local_addr = {};
	int local_len = sizeof(local_addr);
	int r = getsockname(fd, (sockaddr*)&local_addr, &local_len);
	if (r == SOCKET_ERROR) 
	{
		LOG(ERROR) << StringPrintf("getsockname(): %s", LAST_ERROR_MSG);
		return false;
	}
	sockaddr_storage peer_addr = {};
	int peer_len = sizeof(peer_addr);
	r = getpeername(fd, (sockaddr*)&peer_addr, &peer_len);
	if (r == SOCKET_ERROR)
	{
		LOG(ERROR) << StringPrintf("getpeername(): %s", LAST_ERROR_MSG);
		return false;
	}
	switch (local_addr.ss_family)
	{
	case AF_INET:
		{
			const sockaddr_in* locaddr = (const sockaddr_in*)&local_addr;
			const sockaddr_in* remoteaddr = (const sockaddr_in*)&peer_addr;
			if (locaddr->sin_port == remoteaddr->sin_port)
			{
				return locaddr->sin_addr.S_un.S_addr == remoteaddr->sin_addr.S_un.S_addr;
			}
		}
		break;
	case AF_INET6:
		{
			const sockaddr_in6* locaddr = (const sockaddr_in6*)&local_addr;
			const sockaddr_in6* remoteaddr = (const sockaddr_in6*)&peer_addr;
			if (locaddr->sin6_port == remoteaddr->sin6_port)
			{
				return memcmp(&locaddr->sin6_addr, &remoteaddr->sin6_addr, sizeof(in6_addr)) == 0;
			}
		}
		break;
	}
	return false;
}

SOCKET CreateTCPAcceptor(const char* host, const char* port, bool nonblock, bool ipv6)
{
    struct addrinfo* aiList = NULL;
    struct addrinfo* pinfo = NULL;
    struct addrinfo hints = {};
    hints.ai_family = ipv6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host, port, &hints, &aiList);
    if (err != 0)
    {
        LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
        return INVALID_SOCKET;
    }
	SOCKET fd = INVALID_SOCKET;
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
		fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
		if (fd == INVALID_SOCKET)
		{
			LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
			continue;
		}
		int err = bind(fd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
		if (err == SOCKET_ERROR)
		{
			LOG(ERROR) << StringPrintf("%s bind(): %s\n", pinfo->ai_addr, LAST_ERROR_MSG);
			closesocket(fd);
			fd = INVALID_SOCKET;
			continue;
		}
		// set to non-blocking mode
		if (nonblock)
		{
			if (SetNonblock(fd, true) == SOCKET_ERROR)
			{
				closesocket(fd);
				fd = INVALID_SOCKET;
				continue;
			}
		}
		err = listen(fd, SOMAXCONN);
		if (err == SOCKET_ERROR)
		{
			LOG(ERROR) << StringPrintf("listen(): %s\n", LAST_ERROR_MSG);
			closesocket(fd);
			fd = INVALID_SOCKET;
			continue;
		}
		break; // done
    }
    freeaddrinfo(aiList);
    return fd;
}

SOCKET CreateTcpConnector(const char* host, const char* port, bool nonblock, bool ipv6)
{
	struct addrinfo* aiList = NULL;
	struct addrinfo* pinfo = NULL;
	struct addrinfo hints = {};
	hints.ai_family = ipv6 ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int err = getaddrinfo(host, port, &hints, &aiList);
	if (err != 0)
	{
		LOG(ERROR) << StringPrintf("getaddrinfo(): %s:%s, %s.\n", host, port, gai_strerror(err));
		return INVALID_SOCKET;
	}
	SOCKET fd = INVALID_SOCKET;
	for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
	{
		fd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
		if (fd == INVALID_SOCKET)
		{
			LOG(ERROR) << StringPrintf("socket(): %s\n", LAST_ERROR_MSG);
			continue;
		}
		// set to non-blocking mode
		if (nonblock) 
		{
			if (SetNonblock(fd, true) == SOCKET_ERROR)
			{
				closesocket(fd);
				fd = INVALID_SOCKET;
				continue;
			}
		}
		int err = connect(fd, pinfo->ai_addr, (int)pinfo->ai_addrlen);
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
	return fd;
}