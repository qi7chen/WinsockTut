/**
 *  @file   utility.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  macros, functions, and class
 */

#pragma once

#include <Winsock2.h>
#include <string>


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
struct PER_HANDLE_DATA 
{
    WSAOVERLAPPED   overlap_;
    SOCKET          socket_;
    WSABUF          wsbuf_;
    char            buffer_[kDefaultBufferSize];
    OperType        opertype_;
};


// Error description of current thread
#define LAST_ERROR_MSG      GetErrorMessage(::GetLastError()).c_str()


#define CHECK(expr)   if (!(expr)) {                    \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);    \
    throw std::runtime_error(LAST_ERROR_MSG); }
   
// Current date
std::string      Now();

// Description of specified error id
std::string      GetErrorMessage(DWORD dwError);


// Create I/O completion port handle
inline HANDLE   CreateCompletionPort(DWORD dwConcurrency)
{
    return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, dwConcurrency);
}


// Associate device handle to I/O completion port
inline bool AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{    
    HANDLE handle = ::CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0);
    return (handle == hCompletionPort);
}

bool PrintLog(const char* fmt, ...);


#define DEFAULT_HOST    "127.0.0.1"
#define DEFAULT_PORT    "32450"

// Winsock startup and clean
struct WinsockInit
{
public:
    WinsockInit()
    {
        WSADATA data = {};
        CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    }

    ~WinsockInit()
    {
        WSACleanup();
    }

private:
    WinsockInit(const WinsockInit&);
    WinsockInit& operator = (const WinsockInit&);
};
