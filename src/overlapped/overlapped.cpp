/**
 *  @brief:  Overlapped I/O use event
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include "stdafx.h"
#include "../../Common/Mutex.h"
#include "../../Common/Common.h"


#pragma warning(disable: 4127)

using namespace std;


struct SOCK_INFO
{
    SOCKET fd_;
    WSABUF recvbuf_;
    WSABUF sendbuf_;
    WSAOVERLAPPED overlap_;
};


namespace {

const int   BUF_SIZE = 8192;
const int   MAX_CLIENT_COUNT = WSA_MAXIMUM_WAIT_EVENTS;

SOCK_INFO   g_infoList[MAX_CLIENT_COUNT];
int         g_total = 0;
WSAEVENT    g_eventList[MAX_CLIENT_COUNT];
Mutex       g_mutex;

}




unsigned int CALLBACK DoIOFunc(void* param);

SOCK_INFO* AllocateSockInfo(SOCKET sock);
BOOL FreeSockInfo(SOCK_INFO* pInfo);
void DoSend(SOCK_INFO* pInfo);
void OnRecv(SOCK_INFO* pInfo);
void OnClose(SOCK_INFO* pInfo);


// use simple overlapped model
int OverlapWithEvent(const char* host, short port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(host);

    SOCKET sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockListen == INVALID_SOCKET)
    {
        PRINT2LOG(_W("socket() failed"), LAST_ERR_MSG);
        return 0;
    }

    int status = bind(sockListen, (sockaddr*)&addr, sizeof(addr));
    if (status != 0)
    {
        PRINT2LOG(_W("bind() failed"), LAST_ERR_MSG);
        closesocket(sockListen);
        return 0;
    }

    status = listen(sockListen, SOMAXCONN);
    if (status != 0)
    {
        PRINT2LOG(_W("listen() failed"), LAST_ERR_MSG);
        closesocket(sockListen);
        return 0;
    }

    // main thread only do accept
    _beginthreadex(NULL, 0, DoIOFunc, NULL, 0, NULL);

    for (;;)
    {
        SOCKET sockAccept = accept(sockListen, NULL, NULL);
        if (sockAccept == INVALID_SOCKET)
        {
            PRINT2LOG(_W("accept() failed"), LAST_ERR_MSG);
            break;
        }

        SOCK_INFO* pInfo = AllocateSockInfo(sockAccept);
        if (pInfo == NULL)
        {
            closesocket(sockAccept);
            continue;
        }
        
        // deliver a overlap request
        DWORD dwFlag = 0;
        DWORD dwRecvBytes = 0;
        int status = WSARecv(sockAccept, &pInfo->recvbuf_, 1, &dwRecvBytes, &dwFlag, &pInfo->overlap_, NULL);
        if ((status == 0 && dwRecvBytes == 0) || 
            (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING))
        {
            OnClose(pInfo);
            continue;
        }
    }

    return 0;
}





unsigned int CALLBACK DoIOFunc(void* param)
{
    for (;;)
    {
        if (g_total == 0)
        {
            Sleep(1);
            continue;
        }

        int index = WSAWaitForMultipleEvents(g_total, g_eventList, FALSE, INFINITE, FALSE);
        if (index == WSA_WAIT_FAILED || index > g_total)
        {
            PRINT2LOG(_W("WSAWaitForMultipleEvents() failed"), LAST_ERR_MSG);
            break;
        }

        // manually reset
        WSAResetEvent(g_eventList[index]);

        SOCK_INFO* pInfo = &g_infoList[index];
        DWORD dwFlag = 0;
        DWORD dwRecvBytes = 0;
        BOOL status = WSAGetOverlappedResult(pInfo->fd_, &pInfo->overlap_, &dwRecvBytes, FALSE, &dwFlag);
        if (status == TRUE)
        {
            OnRecv(pInfo);
        }
        if (dwRecvBytes == 0)
        {
            OnClose(pInfo);
            continue;
        }

        // deliver next request
        status = WSARecv(pInfo->fd_, &pInfo->recvbuf_, 1, &dwRecvBytes, &dwFlag, &pInfo->overlap_, NULL);
        if ((status == 0 && dwRecvBytes == 0) || 
            (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING))
        {
            OnClose(pInfo);
            continue;
        }
    }

    return 0;
}


void OnRecv(SOCK_INFO* pInfo)
{
    _tprintf(_T("Message from %s:\n\t%s\n"), GetPeerName(pInfo->fd_).c_str(), pInfo->recvbuf_.buf);
    DoSend(pInfo);
}


void DoSend(SOCK_INFO* pInfo)
{
    DWORD dwBytes = pInfo->overlap_.InternalHigh;
    pInfo->sendbuf_.len = dwBytes;
    strncpy_s(pInfo->sendbuf_.buf, BUF_SIZE, pInfo->recvbuf_.buf, dwBytes);    
    DWORD dwBytesSend = 0;
    int status = WSASend(pInfo->fd_, &pInfo->sendbuf_, 1, &dwBytesSend, 0, &pInfo->overlap_, NULL);
    if (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        OnClose(pInfo);
    }
}

void OnClose(SOCK_INFO* pInfo)
{
    assert(pInfo != NULL);
    _tprintf(_T("client %d closed\n"), pInfo->fd_);
    closesocket(pInfo->fd_);
    FreeSockInfo(pInfo);
}

SOCK_INFO* AllocateSockInfo(SOCKET sock)
{
    SOCK_INFO* pInfo = NULL;

    // allocate buffer memory
    char* recvbuf = NULL;
    char* sendbuf = NULL;
    try
    {
        recvbuf = new char[BUF_SIZE];
        sendbuf = new char[BUF_SIZE];
    }
    catch (std::bad_alloc& exp)
    {
        delete [] recvbuf; 
        delete [] sendbuf;
        PRINT2LOG(_W("new operator failed"), LAST_ERR_MSG);
        return NULL;
    }
        
    // allocate event descriptor
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        PRINT2LOG(_W("WSACreateEvent() failed"), LAST_ERR_MSG);
        delete [] sendbuf;
        delete [] recvbuf;
        return NULL;
    }

    SOCK_INFO info = {sock};
    info.recvbuf_.buf = recvbuf;
    info.recvbuf_.len = BUF_SIZE;
    info.sendbuf_.buf = sendbuf;
    info.sendbuf_.len = BUF_SIZE;
    info.overlap_.hEvent = hEvent;    

    // modify global shared resource
    g_mutex.Lock();
    pInfo = &g_infoList[g_total];
    *pInfo = info;
    g_eventList[g_total] = info.overlap_.hEvent;     
    ++g_total;
    g_mutex.UnLock();

    return pInfo;
}


BOOL FreeSockInfo(SOCK_INFO* pInfo)
{
    assert(pInfo != NULL);

    int index = -1;
    for (int i = 0; i < g_total; ++i)
    {
        if (g_infoList[i].fd_ == pInfo->fd_)
        {
            index = i;
            break;
        }
    }

    if (index < 0)
    {
        return FALSE;
    }

    g_mutex.Lock();
    delete [] pInfo->recvbuf_.buf;
    delete [] pInfo->sendbuf_.buf;
    WSACloseEvent(pInfo->overlap_.hEvent);
    g_eventList[index] = WSA_INVALID_EVENT;
    memset(pInfo, 0, sizeof(*pInfo));
    for (int i = index; i < g_total-1; ++i)
    {
        g_infoList[i] = g_infoList[i+1];
        g_eventList[i] = g_eventList[i+1];
    }
    g_eventList[g_total-1] = WSA_INVALID_EVENT;
    memset(&g_infoList[g_total-1], 0, sizeof(*pInfo));
    --g_total;
    g_mutex.UnLock();
    
    return TRUE;
}