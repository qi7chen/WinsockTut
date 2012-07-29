/**
 *  @file   logging.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  logging tools
 */


#pragma once

#include "config.h"

#pragma warning(disable: 4127) // disable 'conditional expression is constant'


_tstring Now();


_tstring GenModuleFileName(const TCHAR* szModule);


_tstring GetErrorMessage(DWORD dwErrorCode);


bool WriteTextToFile(const TCHAR* module, const TCHAR* format, ...);


bool LogErrorText(const TCHAR* module,                  
                  const TCHAR* file, 
                  const TCHAR* func, 
                  size_t line, 
                  const TCHAR* error,
                  const TCHAR* fmt, 
                  ...);



#define LAST_ERROR_MSG      GetErrorMessage(::GetLastError()).c_str()


#define LOG_TEXT(module, fmt, ...)  LogErrorText((module), _T(__FILE__), _T(__FUNCTION__), __LINE__,\
                                            LAST_ERROR_MSG, (fmt), __VA_ARGS__);

#define LOG_TRACE(fmt, ...)     do {LOG_TEXT(_T("trace"), (fmt), __VA_ARGS__);}while(false)
#define LOG_DEBUG(fmt, ...)     do {LOG_TEXT(_T("debug"), (fmt), __VA_ARGS__);}while(false)
#define LOG_WARN(fmt, ...)      do {LOG_TEXT(_T("warning"), (fmt), __VA_ARGS__);}while(false)
#define LOG_INFO(fmt, ...)      do {LOG_TEXT(_T("info"), (fmt), __VA_ARGS__);}while(false)
#define LOG_ERROR(fmt, ...)     do {LOG_TEXT(_T("error"), (fmt), __VA_ARGS__);}while(false)
#define LOG_FATAL(fmt, ...)     do {LOG_TEXT(_T("fatal"), (fmt), __VA_ARGS__);exit(1);}while(false)


#define LOG_PRINT(fmt, ...)     do {_tprintf((fmt), __VA_ARGS__); LOG_DEBUG((fmt), __VA_ARGS__);} while(false)

