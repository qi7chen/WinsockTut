
#include "utility.h"
#include "logging.h"
#include <assert.h>
#include <locale.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <iphlpapi.h>
#include <utility>

#pragma comment(lib, "Iphlpapi")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")





//////////////////////////////////////////////////////////////////////////

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
        LOG_DEBUG(_T("WSAStringToAddress() failed"));
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
            WSAStartup(data.wHighVersion, &data); // use the highest supported version 
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
