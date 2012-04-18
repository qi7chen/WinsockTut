/**
 *  @brief:  Overlapped I/O use complete routine
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include "stdafx.h"
#include "../../Common/Common.h"
#include "../../Common/Mutex.h"


#pragma warning(disable: 4127)

using namespace std;



struct SOCKET_INFO
{
    SOCKET          socket_;
    WSABUF          recvbuf_;
    WSAOVERLAPPED   overlap_;
};

namespace {

const int   BUF_SIZE = 8196;

std::map<SOCKET, SOCKET_INFO>    g_sockinfo;
Mutex           g_mutex;
SOCKET          g_sockAccept;    
int             g_total = 0;

}

unsigned CALLBACK   ThreadFunc(void* param);
SOCKET_INFO*        AllocInfo(SOCKET sock);
BOOL                FreeInfo(SOCKET_INFO* pInfo);
void                OnClose(SOCKET_INFO* pInfo);
void CALLBACK       RecvComplete(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK       SendComplete(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);




int OverlapWithCoRoutine(const char* szhost, short port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(szhost);

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

    printf("listen at %s:%d\n", szhost, port);

    // event object
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        PRINT2LOG(_W("WSACreateEvent() failed"), LAST_ERR_MSG);
        closesocket(sockListen);
        return 0;
    }

    // launch worker thread
    _beginthreadex(NULL, 0, ThreadFunc, hEvent, 0, NULL);

    for (;;)
    {
        g_sockAccept = accept(sockListen, NULL, NULL);
        if (g_sockAccept == INVALID_SOCKET)
        {
            PRINT2LOG(_W("socket() failed"), LAST_ERR_MSG);
            break;
        }
        if (WSASetEvent(hEvent) == FALSE)
        {
            PRINT2LOG(_W("WSASetEvent() failed"), LAST_ERR_MSG);
            closesocket(g_sockAccept);
            g_sockAccept = INVALID_SOCKET;
        }
    }

    return 0;
}



unsigned int CALLBACK ThreadFunc(void* param)
{
    WSAEVENT hEvent = (WSAEVENT)param;
    for (;;)
    {
        int nRet = WSAWaitForMultipleEvents(1, &hEvent, TRUE, INFINITE, TRUE);
        if (nRet == WSA_WAIT_FAILED)
        {
            PRINT2LOG(_W("WSAWaitForMultipleEvents() failed"), LAST_ERR_MSG);
            break;
        }
        if (nRet == WSA_WAIT_IO_COMPLETION)
        {
            continue;
        }
        else
        {
            WSAResetEvent(hEvent);

            SOCKET_INFO* pInfo = AllocInfo(g_sockAccept);
            if (pInfo == NULL)
            {
                closesocket(g_sockAccept);
                g_sockAccept = INVALID_SOCKET;
            }

            printf("%d accepted\n", g_sockAccept);

            DWORD dwFlag = 0;
            DWORD dwRecvBytes = 0;
            int status = WSARecv(g_sockAccept, &pInfo->recvbuf_, 1, &dwRecvBytes, &dwFlag, &pInfo->overlap_, RecvComplete);
            if ((status == 0 && dwRecvBytes == 0) || 
                (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING))
            {
                OnClose(pInfo);        
            }
        }
    }

    return 0;
}



void CALLBACK RecvComplete(DWORD dwError, 
                           DWORD cbTransferred, 
                           LPWSAOVERLAPPED lpOverlapped, 
                           DWORD dwFlags)
{
    SOCKET_INFO* pInfo = (SOCKET_INFO*)lpOverlapped->hEvent;
    if (pInfo == NULL)
    {
        return ;
    }

    if (dwError != 0 || cbTransferred == 0)
    {
        PRINT2LOG(_W("CompleteRoutine failed"), GetErrorMsg(lpOverlapped->Internal));
        OnClose(pInfo);
        return ;
    }

    const wstring& strTime = GetNowStr();
    printf("%s\n", pInfo->recvbuf_.buf);
    wprintf(L"\t%s\n", strTime.c_str());

    pInfo->recvbuf_.len = cbTransferred;
    DWORD dwBytesSend = 0;
    int status = WSASend(pInfo->socket_, &pInfo->recvbuf_, 1, &dwBytesSend, 0, &pInfo->overlap_, SendComplete);
    if (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        OnClose(pInfo);
        return ;
    }

    pInfo->recvbuf_.len = BUF_SIZE;
    DWORD dwRecvBytes = 0;
    status = WSARecv(pInfo->socket_, &pInfo->recvbuf_, 1, &dwRecvBytes, &dwFlags, &pInfo->overlap_, RecvComplete);
    if ((status == 0 && dwRecvBytes == 0) || 
        (status == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING))
    {
        OnClose(pInfo);        
    }
}

void CALLBACK SendComplete(DWORD dwError, 
                           DWORD cbTransferred, 
                           LPWSAOVERLAPPED lpOverlapped, 
                           DWORD dwFlags)
{
    SOCKET_INFO* pInfo = (SOCKET_INFO*)lpOverlapped->hEvent;
    if (pInfo == NULL)
    {
        return ;
    }

    if (dwError != 0 || cbTransferred == 0)
    {
        PRINT2LOG(_W("CompleteRoutine failed"), GetErrorMsg(lpOverlapped->Internal));
        OnClose(pInfo);
        return ;
    }

    const wstring& strTime = GetNowStr();
    wprintf(_W("Send back to %d OK, at %s\n"), pInfo->socket_, strTime.c_str());
}

SOCKET_INFO*  AllocInfo(SOCKET sock)
{
    char* pbuf = NULL;
    try
    {
        pbuf = new char[BUF_SIZE];
    }
    catch (std::bad_alloc& exp)
    {
        PRINT2LOG(_W("socket() failed"), LAST_ERR_MSG);
        return NULL;
    }   
    
    SOCKET_INFO& info = g_sockinfo[sock];
    memset(&info, 0, sizeof(info));
    info.socket_ = sock;
    info.recvbuf_.buf = pbuf;
    info.recvbuf_.len = BUF_SIZE;
    info.overlap_.hEvent = (void*)&info;

    return &info;
}

BOOL    FreeInfo(SOCKET_INFO* pInfo)
{
    assert(pInfo);

    delete pInfo->recvbuf_.buf;    
    g_sockinfo.erase(pInfo->socket_);

    return TRUE;
}

void OnClose(SOCKET_INFO* pInfo)
{
    printf("%d closed\n", pInfo->socket_);
    closesocket(pInfo->socket_);
    FreeInfo(pInfo);
}
