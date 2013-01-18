/**
 *  @file:   cmndef.h
 *  @author: ichenq@gmail.com
 *  @date:   Jul 28, 2012
 *  @brief:  common definition
 *
 */

#pragma once


#ifndef _WIN32
#error "This library is only windows specific!"
#endif


#ifndef _WIN32_WINNT
#define _WIN32_WINNT     0x0502      // Windows 2003 default
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Winsock2.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <iostream>


#if defined(_UNICODE) || defined(UNICODE)
#   define _tstringstream   std::wstringstream
#   define _tstring         std::wstring
#   define _tcout           std::wcout
#else
#   define _tstringstream   std::stringstream
#   define _tstring         std::string
#   define _tcout           std::cout
#endif


enum { BUFE_SIZE = 8192 };


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
    char            buffer_[BUFE_SIZE];
    OperType        opertype_;
};
