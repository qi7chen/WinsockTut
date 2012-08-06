/**
 *  @file:   utility.h
 *  @author: ichenq@gmail.com
 *  @date:   Jul 28, 2012
 *  @brief:  config options
 */

#pragma once


#ifndef _WIN32
#error "This library is only windows specific!"
#endif

// 150021022 is before SP1
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 150030729
#error "This library needs Visual C++ 2008 or later version"
#endif


#ifndef _WIN32_WINNT
#define _WIN32_WINNT     0x0502      // Windows 2003 default
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <assert.h>
#include <tchar.h>
#include <WinSock2.h>
#include <windows.h>


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
