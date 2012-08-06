/**
 *  @brief:  A simple echo server, use winsock Overlapped I/O model
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */



#include "worker.h"
#include "../common/logging.h"
#include "../common/thread.h"


static SOCKET create_listen_socket(const _tstring& strHost, const _tstring& strPort);


int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: $program $host $port"));
        return 1;
    }

    SOCKET socketListen = create_listen_socket(argv[1], argv[2]);
    if (socketListen == INVALID_SOCKET)
    {
        return 1;
    }

    unsigned nWorkerThread = create_thread(worker_routine);

    for (;;)
    {
        SOCKET sockAccept = accept(socketListen, 0, 0);
        if (sockAccept == INVALID_SOCKET)
        {
            break;
        }

        _tprintf(_T("%s, socket %d accepted.\n"), Now().data(), sockAccept);
        send_message_to(nWorkerThread, WM_NEW_SOCKET, sockAccept);
    }

}


SOCKET create_listen_socket(const _tstring& strHost, const _tstring& strPort)
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
    _tprintf(_T("%s, server listen at %s\n"), Now().data(), strAddr.data());
    return sockfd;
}

