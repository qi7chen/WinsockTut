
#include "utility.h"
#include <assert.h>
#include <time.h>
#include <locale.h>
#include <utility>
#include <iostream>
#include <iphlpapi.h>
#include <WinSock2.h>
#include <WS2tcpip.h>


#pragma comment(lib, "Iphlpapi")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")




_tstring GetErrorMessage(DWORD errorcode)
{
    TCHAR szmsg[MAX_PATH];
    DWORD dwLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorcode,
        0, szmsg, MAX_PATH-1, NULL);
    if (dwLen == 0)
    {        
        return GetErrorMessage(::GetLastError()); // find out why we failed
    }
    return _tstring(szmsg, dwLen);
}


_tstring GetDateTimeString(const TCHAR* format, const char* locale)
{    
    assert(format && locale);
    time_t now = time(NULL);
    struct tm st = {};
    errno_t error = localtime_s(&st, &now);
    _locale_t loc = _create_locale(LC_TIME, locale);
    if (loc == 0 || error != 0)
    {
        return _tstring();
    }
    
    TCHAR szmsg[MAX_PATH];
    size_t len = _tcsftime_l(szmsg, _countof(szmsg), format, &st, loc);
    return _tstring(szmsg, len);
}


_tstring  GenerateFullModuleFile(const TCHAR* module)
{
    assert(module);
    _tstring strdate = module + GetDateTimeString(_T("_%Y-%m-%d"), ".936");
    strdate.append(_T(".log"));
    return std::move(strdate);
}


int  SetVarArg(TCHAR* buffer, int buflen, const TCHAR* format, ...)
{
    assert(buffer && format);
    va_list ap;
    va_start(ap, format);
    int count = _vstprintf_s(buffer, buflen, format, ap);
    va_end(ap);
    return count;
}

bool WriteTextToFile(const TCHAR* module, const TCHAR* format, ...)
{
    assert(module && format);
    TCHAR buffer[BUFSIZ];
    va_list ap;
    va_start(ap, format);
    int count = _vstprintf_s(buffer, BUFSIZ, format, ap);
    va_end(ap);

    if (count <= 0)
    {
        return false;
    }

    const _tstring& filename = GenerateFullModuleFile(module);
    FILE* fp = nullptr;
#if defined(_UNICODE) || defined(UNICODE)
    _wfopen_s(&fp, filename.data(), L"a+, ccs=UTF-16LE");
#else
    fopen_s(&fp, filename.data(),  "a+");
#endif
    if (fp == nullptr)
    {
        return false;
    }

    int item_write = fwrite(buffer, sizeof(TCHAR), count, fp);
    fclose(fp);

    return (item_write == count);
}

// write formatted text to specified file
bool LogErrorText(const TCHAR* module,
    const TCHAR* msg, 
    const TCHAR* file, 
    const TCHAR* func, 
    size_t line, 
    size_t errorcode)
{
    assert(module && msg && file && func);
    TCHAR buffer[BUFSIZ];
    const _tstring& strdate = GetDateTimeString(_T("%#c"), ".936");
    const _tstring& errmsg = GetErrorMessage(errorcode);
    const TCHAR* format = _T("Date: %s\nFile: %s\nLine: %d\nFunction: %s()\nMessage: %s\nError: %d, %s\n");
    int count = _stprintf_s(buffer, BUFSIZ, format, strdate.data(), file, line, func, msg, 
        errorcode, errmsg.data());
    if (count > 0)
    {
        return WriteTextToFile(module, buffer);
    }
    return false;
}


_tstring AddressToString(const sockaddr_in& addr)
{
    TCHAR szaddr[MAX_PATH];
    DWORD buflen = _countof(szaddr);
    int error = WSAAddressToString((sockaddr*)&addr, sizeof(addr), NULL, szaddr, &buflen);
    if (error == SOCKET_ERROR)
    {
        LOG_DEBUG(_T("WSAAddressToString() failed"));
        buflen = 0;
    }
    return _tstring(szaddr, buflen);
}


bool StringToAddress(const _tstring& straddr, sockaddr_in* paddr)
{
    assert(paddr);
    sockaddr_in  addr = {};
    int addrlen = sizeof(addr);
    int error = (WSAStringToAddress((LPTSTR)straddr.data(), AF_INET, NULL, 
        (sockaddr*)&addr, &addrlen));
    if (error != 0)
    {
        LOG_DEBUG(_T("WSAStringToAddress() failed"));
    }
    else
    {
        *paddr = addr;
    }
    return error == 0;
}

//  Format mac address
std::string FormateMAC(const unsigned char* pMac, size_t len)
{
    assert(pMac != NULL);
    char szBuf[MAX_PATH] = {};
    for (size_t i = 0, nPos = 0; i < len && nPos < MAX_PATH; ++i)
    {
        sprintf_s(szBuf + nPos, MAX_PATH - nPos, (i + 1 == len ? "%02x" : "%02x-"), pMac[i]);
        nPos += 3;
    }
    return std::string(szBuf);
}


// Get mac address and push_back to a vector
void GetMAC(std::vector<std::string>& vec)
{
    ULONG   nBufLen = 0;
    GetAdaptersInfo(NULL, &nBufLen);
    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(nBufLen);
    if (GetAdaptersInfo(pAdapterInfo, &nBufLen) == ERROR_SUCCESS)
    {
        for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
        {
            vec.push_back(FormateMAC(pAdapter->Address, pAdapter->AddressLength));
        }
    }
    free(pAdapterInfo);
}



//////////////////////////////////////////////////////////////////////////

struct global_init
{
public:
    global_init()
    {
        WSADATA data = {};
        if (WSAStartup(0, &data) == WSAVERNOTSUPPORTED)
        {
            WSAStartup(data.wHighVersion, &data); // use the highest winsock version 
        }

        setlocale(LC_CTYPE, ".936");
        std::locale loc(".936");
        std::wcout.imbue(loc);
    }

    ~global_init()
    {
        WSACleanup();
    }

private:
    global_init(const global_init&);
    global_init& operator = (const global_init&);
};


// initialize winsock
static const global_init  g_init;
