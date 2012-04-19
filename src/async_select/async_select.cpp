
#include "appdef.h"

static bool on_accepted(HWND hwnd, SOCKET sockfd);
static bool on_recv(HWND hwnd, SOCKET sockfd);
static bool on_closed(HWND hwnd, SOCKET sockfd);



bool InitializeServer(HWND hwnd, const _tstring& strAddr)
{
    sockaddr_in addr = {};
    if (!StringToAddress(strAddr, &addr))
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

    // set socket to non-blocking mode automa
    if (WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        closesocket(sockfd);
        return false;
    }
    return true;
}



bool HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error)
{
    if (error)
    {
        LOG_PRINT(_T("an error(%d) encountered!"), error);
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
            on_recv(hwnd, sockfd);
        }
        break;

    case FD_CLOSE:
        {
            on_closed(hwnd, sockfd);
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

    int error = WSAAsyncSelect(socknew, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        LOG_DEBUG(_T("WSAAsyncSelect() failed"));
        closesocket(socknew);
        return false;
    }

    _tstring msg = AddressToString(addr);
    msg += _T(" accpeted at ");
    msg += GetDateTimeString();
    AppendEditText(hwnd, msg.data(), msg.length());
    return true;
}

bool on_recv(HWND hwnd, SOCKET sockfd)
{
    static char databuf[BUFE_SIZE];
    int bytes = recv(sockfd, databuf, BUFE_SIZE, 0);
    if (bytes == SOCKET_ERROR || bytes == 0)
    {
        return HandleNetEvents(hwnd, sockfd, FD_CLOSE, 0);
    }

    AppendEditText(hwnd, (TCHAR*)databuf, bytes/sizeof(TCHAR));
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        return HandleNetEvents(hwnd, sockfd, FD_CLOSE, 0);
    }
    return true;
}

bool on_closed(HWND hwnd, SOCKET sockfd)
{
    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    if (getsockname(sockfd, (sockaddr*)&addr, &addrlen) == SOCKET_ERROR)
    {
        LOG_DEBUG(_T("getsockname() failed"));
    }
    else
    {
        _tstring msg = AddressToString(addr);
        msg += _T(" closed at ");
        msg += GetDateTimeString();
        AppendEditText(hwnd, msg.data(), msg.length());
    }

    return closesocket(sockfd) == 0;
}
