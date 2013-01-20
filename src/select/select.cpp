//  A simple echo server use select model
//  by ichenq@gmail.com 
//  Oct 19, 2011

#include "../common/utility.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <algorithm>

#pragma warning(disable: 4127)
#pragma comment(lib, "ws2_32")



SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort);

// event handlers
void    on_close(SOCKET sockfd, SOCKET* socklist, int* count_ptr);
bool    on_accept(SOCKET sockfd, SOCKET* socklist, int* count_ptr);
bool    on_recv(SOCKET sockfd);


// initialize winsock and other environment
static global_init g_global_init;

// main entry
int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: $program $host $port"));
        return 1;
    }

    SOCKET sockfd = create_listen_socket(argv[1], argv[2]);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }
    
    // total socket descriptor
    SOCKET socklist[FD_SETSIZE] = {};

    // we only concern readable socket set
    FD_SET readset = {}; 

    // how many client sockets are avaliable
    int total_count = 0;

    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset); // add listen socket
        for (int i = 0; i < total_count; ++i)
        {
            FD_SET(socklist[i], &readset); // add client sockets
        }

        // 50 ms
        timeval timeout = {0, 500*1000};
        int nready = select(0, &readset, NULL, NULL, &timeout);
        if (nready == SOCKET_ERROR)
        {
            _tprintf(_T("select() failed, %s"), LAST_ERROR_MSG);
            break;
        }
        if (nready == 0) // time limit expired, timer callback can be handled here
        {            
            continue;
        }

        // check one by one
        for (int i = 0; i < nready; ++i)
        {
            if (FD_ISSET(socklist[i], &readset))
            {
                if (!on_recv(socklist[i]))
                {
                    // recv failed, close the socket
                    on_close(socklist[i], socklist, &total_count);
                }
            }
        }

        // new accepted socket
        if (FD_ISSET(sockfd, &readset))
        {
            on_accept(sockfd, socklist, &total_count);
        }
    }

    closesocket(sockfd); // close the listen socket
    return 0;
}


//////////////////////////////////////////////////////////////////////////
// handlers

bool on_recv(SOCKET sockfd)
{
    char buf[BUFE_SIZE];
    int bytes = recv(sockfd, buf, BUFE_SIZE, 0);
    if (bytes == SOCKET_ERROR)
    {
        _tprintf(_T("recv() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    if (bytes == 0) // gracefully closed
    {
        return false;
    }

    bytes = send(sockfd, buf, bytes, 0);
    if (bytes == SOCKET_ERROR)
    {
        _tprintf(_T("send() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    return true;
}

bool on_accept(SOCKET sockfd, SOCKET* socklist, int* count_ptr)
{
    assert(count_ptr);
    if (*count_ptr == FD_SETSIZE - 1)
    {
        _tprintf(_T("got the 64 limit.\n"));
        return false;
    }

    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    int socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
    if (socknew == INVALID_SOCKET)
    {
        _tprintf(_T("accept() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    // set to non-blocking mode
    ULONG nonblock = 1;
    if (ioctlsocket(socknew, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        _tprintf(_T("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(socknew);
        return false;
    }

    _tprintf(_T("socket %d accepted at %s.\n"), socknew, Now().data());

    socklist[*count_ptr] = socknew;
    (*count_ptr)++;
    return true;
}

void on_close(SOCKET sockfd, SOCKET* socklist, int* count_ptr)
{
    assert(count_ptr);
    _tprintf(_T("socket %d closed at %s.\n"), sockfd, Now().data());

    // find index of this socket
    SOCKET* listend = socklist + (*count_ptr+1);
    bool removed = (std::remove(socklist, listend, sockfd) != listend);
    if (!removed)
    {
        _tprintf(_T("socket %d not found in list.\n"), sockfd);
    }
    else
    {
        closesocket(sockfd);
        (*count_ptr)--;
    }
}


SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    const _tstring& strAddr = strHost + _T(":") + strPort;
    if (!StringToAddress(strAddr, &addr))
    {
        _tprintf(_T("cannot convert '%s' to socket address, %s"), strAddr.data(), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        _tprintf(_T("socket() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        _tprintf(_T("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        _tprintf(_T("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    // set socket to non-blocking mode
    ULONG nonblock = 1;
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        _tprintf(_T("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    _tprintf(_T("server listen at %s, %s.\n"), strAddr.data(), Now().data());
    return sockfd;
}
