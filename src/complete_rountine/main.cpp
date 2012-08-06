/**
 *  @brief:  A simple echo server, use winsock Overlapped I/O model
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */


#include "complete_routine.h"
#include "../common/utility.h"
#include "../common/logging.h"

static const int TIME_OUT = 50;
static SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort);



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
                LOG_DEBUG(_T("accept() failed"));
                break;
            }
        }
        else
        {
            socket_data* data = alloc_data(socknew);
            _tprintf(_T("%s, socket %d accepted.\n"), Now().data(), socknew);
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
        LOG_PRINT(_T("socket() failed, %s"), strAddr.data());
        return INVALID_SOCKET;
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

    _tprintf(_T("%s, server start listen at %s\n"), Now().data(), strAddr.data());
    return sockfd;
}
