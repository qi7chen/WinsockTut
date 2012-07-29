#include "logging.h"
#include <time.h>
#include <string.h>
#include <assert.h>


_tstring Now()
{
    TCHAR szbuf[32] = {};
    struct tm st = {};
    time_t now = time(NULL);
    localtime_s(&st, &now);
    _tcsftime(szbuf, _countof(szbuf), _T("%Y-%m-%d %H:%M:%S"), &st);
    return szbuf;
}

_tstring GenModuleFileName(const TCHAR* szModule)
{
    assert(szModule);
    TCHAR szfmt[MAX_PATH];
    _tcscpy_s(szfmt, _countof(szfmt), szModule);
    _tcscat_s(szfmt, _countof(szfmt), _T("_%Y-%m-%d.log"));

    struct tm st = {};
    time_t now = time(NULL);
    TCHAR szbuf[MAX_PATH] = {0};
    if (localtime_s(&st, &now) == 0 &&
        _tcsftime(szbuf, _countof(szbuf), szfmt, &st) > 0 )
    {
        return szbuf;
    }
    return _tstring();
}


_tstring GetErrorMessage(DWORD dwErrorCode)
{
    TCHAR szMsg[MAX_PATH];
    DWORD dwLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, szMsg, MAX_PATH-1, NULL);
    if (dwLen == 0)
    {        
        return GetErrorMessage(::GetLastError()); // find out why we failed
    }
    return _tstring(szMsg, dwLen);
}


bool WriteTextToFile(const TCHAR* module, const TCHAR* format, ...)
{
    assert(module && format);

    TCHAR buffer[BUFSIZ];
    va_list ap;
    va_start(ap, format);
    int count = _vstprintf_s(buffer, _countof(buffer), format, ap);
    va_end(ap);
    if (count <= 0)
    {
        return false;
    }

    const _tstring& filename = GenModuleFileName(module);
    FILE* fp = NULL;
#if defined(_UNICODE) || defined(UNICODE)
    _wfopen_s(&fp, filename.data(), L"a+, ccs=UTF-16LE");
#else
    fopen_s(&fp, filename.data(),  "a+");
#endif
    if (fp == NULL)
    {
        return false;
    }

    size_t written = fwrite(buffer, sizeof(TCHAR), count, fp);
    fclose(fp);
    return written > 0;
}


bool LogErrorText(const TCHAR* module,                  
                  const TCHAR* file, 
                  const TCHAR* func, 
                  size_t line, 
                  const TCHAR* error,
                  const TCHAR* fmt, 
                  ...)
{
    assert(module && file && func);
    TCHAR buffer[MAX_PATH];
    va_list ap;
    va_start(ap, fmt);
    int count = _vstprintf_s(buffer, _countof(buffer), fmt, ap);
    va_end(ap);
    if (count <= 0)
    {
        return false;
    }

    const _tstring& strDate = Now();
    const TCHAR* format = _T("Date: %s\nFile: %s [%d]\nFunction: %s()\n\
            Error: %s\nMessage: %s\n");
    return WriteTextToFile(module, format, strDate.data(), file, line, 
        func, error, buffer);
}
