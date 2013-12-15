/**
 *  @file   utility.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  类型定义及工具函数集合
 */

#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT     0x0502      // Windows 2003
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <string>
#include <stdexcept>

// 默认缓冲区大小
enum { kDefaultBufferSize = 8192 };

// 套接字操作类型
enum OperType
{
    OperClose,
    OperConnect,
    OperAccept,
    OperSend,
    OperRecv,
    OperDisconnect,
};

// 每个套接字对应的数据结构
struct PER_HANDLE_DATA 
{
    WSAOVERLAPPED   overlap_;
    SOCKET          socket_;
    WSABUF          wsbuf_;
    char            buffer_[kDefaultBufferSize];
    OperType        opertype_;
};


// 错误描述
#define LAST_ERROR_MSG      GetErrorMessage(::GetLastError()).c_str()


#define CHECK(expr)   if (!(expr)) { \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);  \
    throw std::runtime_error(LAST_ERROR_MSG); }
   


// 获取当前时间
std::string      Now();

// 获取当前线程的错误描述
std::string      GetErrorMessage(DWORD dwError);


// 创建I/O完成端口句柄
inline HANDLE   CreateCompletionPort(DWORD dwConcurrency)
{
    return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, dwConcurrency);
}


// 将设备句柄关联到I/O完成端口
inline bool AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{    
    HANDLE handle = ::CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0);
    return (handle == hCompletionPort);
}

// 创建一个线程
DWORD StartThread(unsigned (CALLBACK* routine) (void*), int var);


// Winsock自动初始化
struct WinsockInit
{
public:
    WinsockInit();
    ~WinsockInit();

private:
    WinsockInit(const WinsockInit&);
    WinsockInit& operator = (const WinsockInit&);
};


//
// 用Win32临界区实现的锁对象
//
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
