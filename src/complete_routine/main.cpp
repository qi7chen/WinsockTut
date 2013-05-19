//  A simple echo server use Winsock completion routine(single thread)
//  by ichenq@gmail.com at Oct 19, 2011


#include "../common/utility.h"
#include "complete_routine.h"

#pragma comment(lib, "ws2_32")


SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort);

// initialize winsock
static global_init init;

// alertable thread timeout
const int TIME_OUT = 50;


int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        return 1;
    }

    SOCKET sockfd = create_listen_socket(argv[1], argv[2]);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        sockaddr_in addr = {};
        int addrlen = sizeof(addr);
        SOCKET socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
        if (socknew == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {   
                _tprintf(_T("accept() failed, %s"), LAST_ERROR_MSG);
                break;
            }
        }
        else
        {
            socket_data* data = alloc_data(socknew);
            _tprintf(_T("socket %d accepted at %s.\n"), socknew, Now().data());
            if (data)
            {
                post_recv_request(data);
            }
        }

        ::SleepEx(TIME_OUT, TRUE); // make this thread alertable
    }

    return 0;
}


SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    const _tstring& strAddr = strHost + _T(":") + strPort;
    if (!StringToAddress(strAddr, &addr))
    {
        return INVALID_SOCKET;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        _tprintf(_T("socket() failed '%s', %s"), strAddr.data(), LAST_ERROR_MSG);
        return INVALID_SOCKET;
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

    _tprintf(_T("server start listen [%s] at %s.\n"), strAddr.data(), Now().data());
    return sockfd;
}
