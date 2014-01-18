/**
 *  @file   async_select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by WSAAsyncSelect()
 *			
 */

#include "appdef.h"
#include <set>
#include <algorithm>

namespace {
    std::set<SOCKET>  g_socketList;
}


bool InitializeServer(HWND hwnd, const char* host, int port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);   
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
		fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
		fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
		fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms741540(v=vs.85).aspx
    //
    // The WSAAsyncSelect function automatically sets socket s to nonblocking mode, 
    // regardless of the value of lEvent. 
    if (WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
		fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return false;
    }

    fprintf(stdout, ("server listen at %s:%d, %s.\n"), host, port, Now().data());
    g_socketList.insert(sockfd);

    return true;
}

bool on_accepted(HWND hwnd, SOCKET sockfd)
{
    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    SOCKET socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
    if (socknew == INVALID_SOCKET)
    {
        fprintf(stderr, ("accpet() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    int error = WSAAsyncSelect(socknew, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(socknew);
        return false;
    }

    g_socketList.insert(socknew);
    fprintf(stdout, ("socket %d accepted at %s.\n"), socknew, Now().data());
    return true;
}

void on_closed(SOCKET sockfd)
{
    closesocket(sockfd);
    g_socketList.erase(sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now().data());
}

bool on_recv(SOCKET sockfd)
{
    char databuf[kDefaultBufferSize];
    int bytes = recv(sockfd, databuf, kDefaultBufferSize, 0);
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


void CloseServer()
{
    int count = g_socketList.size();
    for_each(g_socketList.begin(), g_socketList.end(), on_closed);
    g_socketList.clear();
    fprintf(stdout, ("server[%d] closed at %s.\n"), count, Now().data());
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