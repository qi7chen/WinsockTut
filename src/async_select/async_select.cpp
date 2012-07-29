
#include "appdef.h"
#include "../common/logging.h"
#include <set>
#include <algorithm>


static bool on_accepted(HWND hwnd, SOCKET sockfd);
static bool on_recv(SOCKET sockfd);
static void on_closed(SOCKET sockfd);


static std::set<SOCKET>  g_socketList;


bool InitializeServer(HWND hwnd, const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    if (!StringToAddress(strHost + _T(":") + strPort, &addr))
    {
        return false;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        LOG_PRINT(_T("socket() failed"));
        return false;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("bind() failed"));
        closesocket(sockfd);
        return false;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("listen() failed"));
        closesocket(sockfd);
        return false;
    }

    // set the socket to non-blocking mode automatically
    if (WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        closesocket(sockfd);
        return false;
    }

    g_socketList.insert(sockfd);
    return true;
}

void CloseServer()
{
    size_t count = g_socketList.size();
    for_each(g_socketList.begin(), g_socketList.end(), on_closed);
    g_socketList.clear();
    TCHAR szmsg[MAX_PATH];
    int len = _stprintf_s(szmsg, MAX_PATH, _T("%s, server closed with %d sockets"), Now().data(), count);
    AppendLogText(szmsg, len);
}


bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error)
{
    if (error)
    {
        LOG_PRINT(_T("Error encountered, ID: %d\n."), error);
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
        LOG_DEBUG(_T("accpet() failed"));
        return false;
    }

    // set the socket to non-blocking mode automatically
    int error = WSAAsyncSelect(socknew, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        LOG_DEBUG(_T("WSAAsyncSelect() failed"));
        closesocket(socknew);
        return false;
    }

    g_socketList.insert(socknew);
    _tstring msg = Now() + _T(", socket ") + ToString(socknew) + _T(" accepted.\n");
    AppendLogText(msg.data(), msg.length());
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

    AppendLogText((TCHAR*)databuf, bytes/sizeof(TCHAR));
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
    _tstring msg = Now() + _T(", socket ") + ToString(sockfd) + _T(" closed.\n");
    AppendLogText(msg.data(), msg.length());    
}
