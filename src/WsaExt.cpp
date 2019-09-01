// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "WsaExt.h"
#include "Common/Error.h"
#include "Common/Logging.h"


bool WsaExt::intialized_ = false;
LPFN_CONNECTEX WsaExt::ConnectEx = NULL;
LPFN_ACCEPTEX WsaExt::AcceptEx = NULL;
LPFN_ACCEPTEX WsaExt::DisconnectEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS WsaExt::GetAcceptExSockaddrs = NULL;

WsaExt::WsaExt()
{
    Init(INVALID_SOCKET);
}

void WsaExt::Init(SOCKET sock)
{
    if (intialized_)
        return;

    SOCKET fd = sock;
    if (fd == INVALID_SOCKET)
    {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPv4
    }

    GUID guid_connectex = WSAID_CONNECTEX;
    GUID guid_acceptex = WSAID_ACCEPTEX;
    GUID guid_disconnectex = WSAID_DISCONNECTEX;
    GUID guid_getacceptexaddr = WSAID_GETACCEPTEXSOCKADDRS;

    int err = 0;
    DWORD bytes = 0;

    // ConnectEx
    err = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connectex,
        sizeof(guid_connectex), &ConnectEx, sizeof(ConnectEx), &bytes, NULL, NULL);
    CHECK(err != SOCKET_ERROR) << "WSAIoctl: ConnectEx, " << LAST_ERROR_MSG;

    // AcceptEx
    err = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_acceptex,
        sizeof(guid_acceptex), &AcceptEx, sizeof(AcceptEx), &bytes, NULL, NULL);
    CHECK(err != SOCKET_ERROR) << "WSAIoctl: AcceptEx, " << LAST_ERROR_MSG;

    // DisconnectEx
    err = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_disconnectex,
        sizeof(guid_disconnectex), &DisconnectEx, sizeof(DisconnectEx), &bytes, NULL, NULL);
    CHECK(err != SOCKET_ERROR) << "WSAIoctl: DisconnectEx, " << LAST_ERROR_MSG;

    // GetAcceptExSockaddrs
    err = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_getacceptexaddr,
        sizeof(guid_getacceptexaddr), &GetAcceptExSockaddrs, sizeof(GetAcceptExSockaddrs), &bytes, NULL, NULL);
    CHECK(err != SOCKET_ERROR) << "WSAIoctl: GetAcceptExSockaddrs, " << LAST_ERROR_MSG;

    if (sock == INVALID_SOCKET)
    {
        closesocket(fd);
    }
    intialized_ = true;
}