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


#define CHECK(expr)   if (!(expr)) { \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);  \
    throw std::runtime_error(LAST_ERROR_MSG); }
   

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

// Start a thread
DWORD StartThread(unsigned (CALLBACK* routine) (void*), int var);


// Winsock startup and clean
struct WinsockInit
{
public:
    WinsockInit();
    ~WinsockInit();

private:
    WinsockInit(const WinsockInit&);
    WinsockInit& operator = (const WinsockInit&);
};



// Mutex with critial section
class Mutex
{
public:
    Mutex() 
    {
        ::InitializeCriticalSection(&cs_);
    }
    ~Mutex()
    {
        ::DeleteCriticalSection(&cs_);
    }

    bool TryLock() 
    {
        return (::TryEnterCriticalSection(&cs_) == TRUE);
    }

    void Lock() 
    {
        ::EnterCriticalSection(&cs_);
    }

    void UnLock() 
    {
        ::LeaveCriticalSection(&cs_);
    }

private:
    Mutex(const Mutex&);
    Mutex& operator = (const Mutex&);

    CRITICAL_SECTION    cs_;
};

// RAII
template <typename Lockable>
class ScopedMutexGuard
{
public:
    explicit ScopedMutexGuard(Lockable& lock)
        : lock_(lock)
    {
        lock_.Lock();
    }

    ~ScopedMutexGuard() 
    {
        lock_.UnLock();
    }

private:
    ScopedMutexGuard(const ScopedMutexGuard&);
    ScopedMutexGuard& operator = (const ScopedMutexGuard&);

    Lockable&  lock_;
};
