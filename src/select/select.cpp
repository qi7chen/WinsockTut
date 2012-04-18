

#include "../common/utility.h"
#include <WinSock2.h>
#include <WS2tcpip.h>



SOCKET  create_server_socket(const _tstring& strAddr);

void    on_close(SOCKET sockfd, SOCKET* socklist, int* count);
bool    on_accept(SOCKET sockfd, SOCKET* socklist, int* count);
bool    on_recv(SOCKET sockfd);




int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        exit(1);
    }

    _tstring host = argv[1];
    _tstring port = argv[2];

    SOCKET sockfd = create_server_socket(host + _T(":") + port);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    int total_count = 0;
    SOCKET socklist[FD_SETSIZE] = {};
    FD_SET readset = {};

    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset);
        for (int i = 0; i < total_count; ++i)
        {
            FD_SET(socklist[i], &readset);
        }

        // 50 ms
        timeval timeout = {0, 500*1000};
        int nready = select(0, &readset, NULL, NULL, &timeout);
        if (nready == SOCKET_ERROR)
        {
            LOG_PRINT(_T("select() failed"));
            break;
        }
        if (nready == 0)
        {
            // handle timers here
            continue;
        }

        // check one by one
        for (int i = 0; i < nready; ++i)
        {
            if (FD_ISSET(socklist[i], &readset))
            {
                if (!on_recv(socklist[i]))
                {
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

    closesocket(sockfd);
    return 0;
}


//
// handlers
//


bool    on_recv(SOCKET sockfd)
{
    char buf[BUFE_SIZE];
    int bytes = recv(sockfd, buf, BUFE_SIZE, 0);
    if (bytes == SOCKET_ERROR)
    {
        LOG_PRINT(_T("recv() failed."));
        return false;
    }
    if (bytes == 0) // closed
    {
        return false;
    }

    bytes = send(sockfd, buf, bytes, 0);
    if (bytes == SOCKET_ERROR)
    {
        LOG_PRINT(_T("send() failed."));
        return false;
    }

    return true;
}

bool on_accept(SOCKET sockfd, SOCKET* socklist, int* count)
{
    if (*count == FD_SETSIZE-1)
    {
        LOG_PRINT(_T("got the select 64 limit"));
        return false;
    }

    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    int socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
    if (socknew == INVALID_SOCKET)
    {
        LOG_PRINT(_T("accept() failed"));
        return false;
    }

    // set socket to non-blocking mode
    ULONG nonblock = 1;
    if (ioctlsocket(socknew, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        LOG_PRINT(_T("ioctlsocket() failed"));
        closesocket(socknew);
        return false;
    }

    socklist[*count] = socknew;
    (*count)++;
    return true;
}

void    on_close(SOCKET sockfd, SOCKET* socklist, int* count)
{
    _tprintf(_T("%d closed at %s\n"), sockfd, GetDateTimeString().data());
    int index = -1;
    for (int i = 0; i < *count; ++i)
    {
        if (socklist[i] == sockfd)
        {
            index = i;
            break;
        }
    }
    
    if (index < 0)
    {
        LOG_PRINT(_T("socket %d not found in list"), sockfd);
        return ;
    }

    closesocket(sockfd);
    for (int i = index; i < *count; ++i)
    {
        socklist[i] = socklist[i+1];
    }
    (*count)--;
}


SOCKET  create_server_socket(const _tstring& strAddr)
{
    sockaddr_in addr = {};
    StringToAddress(strAddr, &addr);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        LOG_PRINT(_T("socket() failed, %s"), strAddr.data());
        return 0;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("bind() failed"));
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("listen() failed"));
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    // set socket to non-blocking mode
    ULONG nonblock = 1;
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        LOG_PRINT(_T("ioctlsocket() failed"));
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    _tprintf(_T("server listen at %s\n"), strAddr.data());
    return sockfd;
}