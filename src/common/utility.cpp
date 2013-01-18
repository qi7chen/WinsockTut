
#include "utility.h"
#include "logging.h"
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


bool StringToAddress(const _tstring& strAddr, sockaddr_in* paddr)
{
    assert(paddr);
    sockaddr_in  addr = {};
    int addrlen = sizeof(addr);
    int error = (WSAStringToAddress(const_cast<LPTSTR>(strAddr.data()), AF_INET, NULL, 
        (sockaddr*)&addr, &addrlen));
    if (error != 0)
    {
        return false;
    }
    *paddr = addr;
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


