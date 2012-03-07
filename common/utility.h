/**
 *  @file:   utility.h
 *  @brief:  common utilities
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once


#include <tchar.h>
#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <WinSock2.h>
#include <string>
#include <sstream>


#pragma warning(disable: 4127)



#if defined(_UNICODE) || defined(UNICODE)
#   define _tstringstream   std::wstringstream
#   define _tstring         std::wstring
#   define _tcout           std::wcout
#else
#   define _tstringstream   std::stringstream
#   define _tstring     std::string
#   define _tcout       std::cout
#endif



#define LOG_DEBUG(fmt, ...)     WriteTextToFile(_T("debug"), (fmt), __VA_ARGS__)


#define LOG_ERROR_ENV(msg)      do { \
                                LogErrorText((msg), _T(__FILE__), __LINE__, _T(__FUNCTION__), ::GetLastError()); \
                                }while(false)


#define LOG_ERROR_MSG(fmt, ...) do { \
                                TCHAR _buffer[BUFSIZ]; \
                                SetVarArg(_buffer, BUFSIZ, fmt, __VA_ARGS__); \
                                LOG_ERROR_ENV(_buffer); }while(false)


#define LOG_PRINT(fmt, ...)     do {_tprintf((fmt), __VA_ARGS__); LOG_ERROR_MSG((fmt), __VA_ARGS__);} while(false)

template <typename T>
_tstring    ToString(const T& obj)
{
    _tstringstream strm;
    strm << obj;
    return std::move(strm.str());
}

// get formatted message string by specified error code
_tstring    GetErrorMessage(DWORD errorcode);


// converts a sockaddr_in structure into a human-readable string 
_tstring	AddressToString(const sockaddr_in& addr);


// converts a numeric string to a sockaddr_in structure
bool        StringToAddress(const _tstring& straddr, sockaddr_in* paddr);


_tstring    GenerateFullModuleFile(const TCHAR* strmodule);


// get a date time string with specified locale setting
_tstring    GetDateTimeString(const TCHAR* format = _T("%#c"), const char* locale = ".936");


int         SetVarArg(TCHAR* buffer, int buflen, const TCHAR* format, ...);

// write formatted text to specified file
bool        WriteTextToFile(const TCHAR* module, const TCHAR* format, ...);


// log the information message of specified error
bool        LogErrorText(const TCHAR* msg, 
                        const TCHAR* file, 
                        size_t line, 
                        const TCHAR* func, 
                        size_t errorcode);



enum { BUFE_SIZE = 8192 };


enum OperType
{
    OperConnect, 
    OperAccept, 
    OperSend, 
    OperRecv, 
    OperDisconnect,
    OperClose,
};


struct PER_HANDLE_DATA 
{
    WSAOVERLAPPED   overlap_;
    SOCKET          socket_;
    WSABUF          wsbuf_;
    char            buffer_[BUFE_SIZE];
    OperType        opertype_;
};

inline HANDLE   CreateCompletionPort(size_t concurrency)
{
    return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrency);
}


inline bool     AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{
    assert(hCompletionPort != INVALID_HANDLE_VALUE);
    return (::CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0) == hCompletionPort);
}
