/**
 *  @file:   utility.h
 *  @brief:  common utilities
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#pragma once


#include <tchar.h>
#include <stdio.h>
#include <assert.h>
#include <WinSock2.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>


#pragma warning(disable: 4127)

#include <boost/shared_ptr.hpp>
#if _MSC_VER_ < 1600
#   ifndef BOOST_BIND
#   include <boost/bind.hpp>
#   include <boost/shared_ptr.hpp>
#   define BIND     boost::bind
    using boost::shared_ptr;
#   endif
#else
#   define BIND     std::bind
    using std::shared_ptr;
#endif




#if defined(_UNICODE) || defined(UNICODE)
#   define _tstringstream   std::wstringstream
#   define _tstring         std::wstring
#   define _tcout           std::wcout
#else
#   define _tstringstream   std::stringstream
#   define _tstring         std::string
#   define _tcout           std::cout
#endif


//////////////////////////////////////////////////////////////////////////
// common utilities

#define LAST_ERROR_MSG          GetErrorMessage(::GetLastError()).c_str()


#define LOG_TEXT(module, fmt, ...)  \
                do { \
                TCHAR _buffer[MAX_PATH]; \
                SetVarArg(_buffer, MAX_PATH, (fmt), __VA_ARGS__); \
                LogErrorText((module), _buffer, _T(__FILE__), _T(__FUNCTION__), __LINE__, ::GetLastError()); \
                }while (false)

#define LOG_ERROR(fmt, ...)     LOG_TEXT(_T("error"), (fmt), __VA_ARGS__)
#define LOG_DEBUG(fmt, ...)     LOG_TEXT(_T("debug"), (fmt), __VA_ARGS__)
#define LOG_WARNING(fmt, ...)   LOG_TEXT(_T("warning"), (fmt), __VA_ARGS__)


#define LOG_PRINT(fmt, ...)     do {_tprintf((fmt), __VA_ARGS__); LOG_DEBUG((fmt), __VA_ARGS__);} while(false)



template <typename T>
_tstring    ToString(const T& obj)
{
    _tstringstream strm;
    strm << obj;
    return strm.str();
}


// trim string
template <typename StringT>
StringT&    TrimLeft(StringT& str)
{
    StringT::iterator iter = std::find_if(str.begin(), str.end(), 
        std::not1(std::ptr_fun(isspace)));
    str.erase(str.begin(), iter);
    return str;
}


template <typename StringT>
StringT&    TrimRight(StringT& str)
{
    StringT::reverse_iterator iter = std::find_if(str.rbegin(), str.rend(), 
        std::not1(std::ptr_fun(isspace)));
    str.erase(iter.base(), str.end());
    return str;
}

template <typename StringT>
StringT&    TrimString(StringT& str)
{
    return TrimLeft(TrimRight(str));
}



// get formatted message string by specified error code
_tstring    GetErrorMessage(DWORD errorcode);



_tstring    GenerateFullModuleFile(const TCHAR* strmodule);


// get a date time string with specified locale setting
_tstring    GetDateTimeString(const TCHAR* format = _T("%#c"));


int         SetVarArg(TCHAR* buffer, int buflen, const TCHAR* format, ...);

// write formatted text to specified file
bool        WriteTextToFile(const TCHAR* module, const TCHAR* format, ...);


// log the information message of specified error
bool        LogErrorText(const TCHAR* module, 
                        const TCHAR* msg, 
                        const TCHAR* file, 
                        const TCHAR* func, 
                        size_t line,                          
                        size_t errorcode);



//////////////////////////////////////////////////////////////////////////
// winsock specified utilities


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


//////////////////////////////////////////////////////////////////////////
// network utilities

// converts a sockaddr_in structure into a human-readable string 
_tstring	AddressToString(const sockaddr_in& addr);


// converts a numeric string to a sockaddr_in structure
bool        StringToAddress(const _tstring& straddr, sockaddr_in* paddr);






// format mac address
std::string FormateMAC(const BYTE* pMac, size_t len);


// get mac address and push back to a vector
void    GetMAC(std::vector<std::string>& vec);


// 
#if _WIN32_WINNT < 0x0600
int         inet_pton(int af, const char* src, void* dst);
const char* inet_ntop(int af, const void* src, char* dst, size_t size);
#endif

