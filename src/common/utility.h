/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License.
 * See accompanying files LICENSE.
 */

#pragma once

#include <stdlib.h>
#include <Windows.h>
#include <WinSock2.h>

#define TLS     __declspec(thread)
#define INLINE  __inline


/* default I/O buffer size */
enum { kDefaultBufferSize = 8192 };

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

/* per-handle data */
typedef struct _PER_HANDLE_DATA 
{
    WSAOVERLAPPED   overlap;
    SOCKET          socket;
    WSABUF          wsbuf;
    char            buffer[kDefaultBufferSize];
    enum OperType   opertype;
}PER_HANDLE_DATA;


/* current date */
const char*     Now();

/* description of specified error id */
const char*     GetErrorMessage(DWORD dwError);


/* create I/O completion port handle */
INLINE HANDLE   CreateCompletionPort(DWORD dwConcurrency)
{
    return CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, dwConcurrency);
}

/* associate device handle to I/O completion port */
INLINE int AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{    
    HANDLE handle = CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0);
    return (handle == hCompletionPort);
}

/* error description of current thread */
#define LAST_ERROR_MSG      GetErrorMessage(GetLastError())


#define CHECK(expr)   if (!(expr)) {                    \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);    \
    abort(); }

#define DEFAULT_HOST    "127.0.0.1"
#define DEFAULT_PORT    "32450"



