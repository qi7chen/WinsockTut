
#include "utility.h"
#include <assert.h>
#include <locale.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <iphlpapi.h>
#include <utility>


//////////////////////////////////////////////////////////////////////////
_tstring Now()
{
    TCHAR szbuf[32] = {};
    struct tm st = {};
    time_t now = time(NULL);
    localtime_s(&st, &now);
    _tcsftime(szbuf, _countof(szbuf), _T("%Y-%m-%d %H:%M:%S"), &st);
    return szbuf;
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

_tstring AddressToString(const sockaddr_in& addr)
{
    TCHAR szaddr[MAX_PATH];
    DWORD buflen = _countof(szaddr);
    int error = WSAAddressToString((sockaddr*)&addr, sizeof(addr), NULL, szaddr, &buflen);
    if (error == SOCKET_ERROR)
    {
        buflen = 0;
    }
    return _tstring(szaddr);
}


bool StringToAddress(const _tstring& strAddr, sockaddr_in* pAddr)
{
    assert(pAddr);
    sockaddr_in  addr = {};
    int addrlen = sizeof(addr);
    int error = (WSAStringToAddress(const_cast<LPTSTR>(strAddr.data()), AF_INET, NULL, 
        (sockaddr*)&addr, &addrlen));
    if (error != 0)
    {
        return false;
    }
    *pAddr = addr;
    return true;
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

bool send_message_to(unsigned thread_id, 
                     unsigned msg, 
                     unsigned param1 /* = 0 */, 
                     long param2 /* = 0 */)
{
    BOOL status = ::PostThreadMessage(thread_id, msg, param1, param2);
    return (status == TRUE);
}

//////////////////////////////////////////////////////////////////////////
global_init::global_init()
{
    WSADATA data = {};
    if (WSAStartup(0, &data) == WSAVERNOTSUPPORTED)
    {
        WSAStartup(data.wHighVersion, &data); // use the highest supported version 
    }

    setlocale(LC_CTYPE, ".936");
    std::locale loc(".936");
    std::wcout.imbue(loc);
}

global_init::~global_init()
{
    WSACleanup();
}


