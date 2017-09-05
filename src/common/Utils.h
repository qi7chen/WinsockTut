/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <stdlib.h>
#include <Windows.h>
#include <WinSock2.h>

/* default I/O buffer size */
#define  DEFAULT_BUFFER_SIZE 8192

#define MAX_PAYLOAD  8192

/* type of I/O operation */
enum OperType
{
    OperClose,          /* socket is closed, default state */
    OperConnect,        /* connecting to another peer */
    OperAccept,         /* accept new client connection */
    OperSend,           /* sending data */
    OperRecv,           /* receive data */
    OperDisconnect,     /* disconnect an socket */
};


/// description of specified error id 
const char*  GetErrorMessage(DWORD dwError);

// error description of current thread
#define LAST_ERROR_MSG   GetErrorMessage(GetLastError())

#define CHECK(expr)   if (!(expr)) {                    \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);    \
    abort(); }

// Auto initialize winsock
struct WinsockAutoInit
{
    WinsockAutoInit()
    {
        WSADATA data;
        int r = WSAStartup(MAKEWORD(2, 2), &data);
        CHECK(r == 0);
    }

    ~WinsockAutoInit()
    {
        WSACleanup();
    }
};

// current date
const char* Now(void);

// Create an acceptor socket file descriptor
SOCKET CreateTCPAcceptor(const char* host, const char* port, bool nonblock);
