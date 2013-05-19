//  A simple echo server use asynchrounous select model
//  by ichenq@gmail.com 
//  Oct 19, 2011

#include "appdef.h"
#include <set>
#include <algorithm>


static bool on_accepted(HWND hwnd, SOCKET sockfd);
static bool on_recv(SOCKET sockfd);
static void on_closed(SOCKET sockfd);


static std::set<SOCKET>  g_socketList;


bool InitializeServer(HWND hwnd, const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    _tstring strAddress = strHost + _T(":") + strPort;
    CHECK (StringToAddress(strAddress, &addr));


    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
		//PrintLog(_T("socket() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
		//PrintLog(_T("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
		//PrintLog(_T("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    // set the socket to non-blocking mode automatically
    if (WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
		//PrintLog(_T("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    g_socketList.insert(sockfd);
    return true;
}

void CloseServer()
{
    //size_t count = g_socketList.size();
    for_each(g_socketList.begin(), g_socketList.end(), on_closed);
    g_socketList.clear();
    //PrintLog(_T("server[%d] closed at %s.\n"), count, Now().data());
}


bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error)
{
    if (error)
    {
        on_closed(sockfd);
        return false;
    }

    switch(event)
    {
    case FD_ACCEPT:
        {
            on_accepted(hwnd, sockfd);
        }
        break;

    case FD_READ:
        {
            on_recv(sockfd);
        }
        break;

    case FD_WRITE:
        {
            // nothing
        }
        break;

    case FD_CLOSE:
        {
            on_closed(sockfd);
        }
        break;

    default:
        break;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////

bool on_accepted(HWND hwnd, SOCKET sockfd)
{
    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    SOCKET socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
    if (socknew == INVALID_SOCKET)
    {
        //PrintLog(_T("accpet() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    // set the socket to non-blocking mode automatically
    int error = WSAAsyncSelect(socknew, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        //PrintLog(_T("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(socknew);
        return false;
    }

    g_socketList.insert(socknew);
    //PrintLog(_T("socket %d accepted at %s.\n"), socknew, Now().data());
    return true;
}

bool on_recv(SOCKET sockfd)
{
    static char databuf[BUFE_SIZE];
    int bytes = recv(sockfd, databuf, BUFE_SIZE, 0);
    if (bytes == SOCKET_ERROR || bytes == 0)
    {
        on_closed(sockfd);
        return false;
    }
    
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        on_closed(sockfd);
        return false;
    }
    return true;
}

void on_closed(SOCKET sockfd)
{
    closesocket(sockfd);
    g_socketList.erase(sockfd);
    //PrintLog(_T("socket %d closed at %s.\n"), sockfd, Now().data());    
}
