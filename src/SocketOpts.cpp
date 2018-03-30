// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "SocketOpts.h"
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include <WS2tcpip.h>


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
        if (r == SOCKET_ERROR)
        {
            r = WSAGetLastError();
            if (r != WSAEWOULDBLOCK)
            {
                LOG(ERROR) << r << LAST_ERROR_MSG;
                return -1;
            }
            break;
        }
        break;  // EOF
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
