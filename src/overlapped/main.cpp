//  A simple echo server use winsock Overlapped I/O
//  by ichenq@gmail.com at Oct 19, 2011

#include <process.h>
#include "overlap_worker.h"
#include "../common/utility.h"

#pragma comment(lib, "ws2_32")


SOCKET create_listen_socket(const _tstring& strHost, const _tstring& strPort);

// worker thread entry
unsigned CALLBACK NativeThreadFunc(void* param);

// initialize winsock
static global_init init;



// main entry
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

    unsigned nWorkderID = 0;
    _beginthreadex(NULL, 0, NativeThreadFunc, NULL, 0, &nWorkderID);

    for (;;)
    {
        SOCKET sockAccept = accept(socketListen, 0, 0);
        if (sockAccept == INVALID_SOCKET)
        {
            _tprintf(_T("accept() failed, %s"), LAST_ERROR_MSG);
            break;
        }

        _tprintf(_T("socket %d accepted at %s.\n"), sockAccept, Now().data());

        // let the worker thread to handle this client connection
        send_message_to(nWorkderID, WM_NEW_SOCKET, sockAccept);
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
    _tprintf(_T("server start listen [%s] at %s.\n"), strAddr.data(), Now().data());
    return sockfd;
}

unsigned CALLBACK NativeThreadFunc(void* param)
{    
    try
    {
        worker_routine();
    }
    catch(...)
    {
        return 1;
    }
    return 0;
}