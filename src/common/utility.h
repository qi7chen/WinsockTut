/**
 *  @file   utility.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  utility macros and functions
 */

#pragma once

#include <stdlib.h>
#include <Windows.h>
#include <WinSock2.h>

#define TLS     __declspec(thread)
#define INLINE  __inline

// Default I/O buffer size
enum { kDefaultBufferSize = 8192 };

// Type of I/O operation
enum OperType
{
    OperClose,
    OperConnect,
    OperAccept,
    OperSend,
    OperRecv,
    OperDisconnect,
};

// per-handle data
typedef struct _PER_HANDLE_DATA 
{
    WSAOVERLAPPED   overlap_;
    SOCKET          socket_;
    WSABUF          wsbuf_;
    char            buffer_[kDefaultBufferSize];
    enum OperType   opertype_;
}PER_HANDLE_DATA;


// Current date
const char*     Now();

// Description of specified error id
const char*     GetErrorMessage(DWORD dwError);


// Create I/O completion port handle
INLINE HANDLE   CreateCompletionPort(DWORD dwConcurrency)
{
    return CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, dwConcurrency);
}

// Associate device handle to I/O completion port
INLINE int AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{    
    HANDLE handle = CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0);
    return (handle == hCompletionPort);
}

// Error description of current thread
#define LAST_ERROR_MSG      GetErrorMessage(GetLastError())


#define CHECK(expr)   if (!(expr)) {                    \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);    \
    abort(); }

#define DEFAULT_HOST    "127.0.0.1"
#define DEFAULT_PORT    "32450"



