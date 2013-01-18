
//  A simple echo server use fundamental socket api
//  by ichenq@gmail.com at Oct 19, 2011

#include "../common/utility.h"
#include "../common/logging.h"
#include <WS2tcpip.h>   // addrinfo
#include <process.h>    // _beginthreadex


#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


void        print_usage();
SOCKET      create_listen_socket(const TCHAR* host, const TCHAR* port);
unsigned CALLBACK   handle_client(void* param);


// initialize winsock and other environment
global_init g_global_init;


// main entry
int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        print_usage();
        return 1;
    }

    SOCKET sockfd = create_listen_socket(argv[1], argv[2]);
    if (sockfd == INVALID_SOCKET)
    {
        print_usage();
        return 1;
    }

    for (;;)
    {
        sockaddr_in addr = {};
        int len = sizeof(addr);
        SOCKET sock_accept = accept(sockfd, (sockaddr*)&addr, &len);
        if (sock_accept == INVALID_SOCKET)
        {
            _tprintf(_T("listen() failed, program exit."));
            break;
        }

        _tprintf(_T("socket %d accepted at %s.\n"), sock_accept, Now().data());

        // one thread per connection
        _beginthreadex(NULL, 0, handle_client, (void*)sock_accept, 0, NULL);   
    }

    return 0;
}


// run in client thread
unsigned CALLBACK handle_client(void* param)
{
    SOCKET sockfd = (SOCKET)param;
    char databuf[BUFSIZ];
    for (;;)
    {
        int bytes_read = recv(sockfd, databuf, BUFSIZ, 0);
        if (bytes_read == SOCKET_ERROR)
        {
            _tprintf(_T("recv() failed, thread for socket %d ends.\n"), sockfd);
            break;
        }
        else if (bytes_read == 0) // connection gracefully closed
        {
            break;
        }

        // write back what we read
        int bytes_send = send(sockfd, databuf, bytes_read, 0);
        if (bytes_send == SOCKET_ERROR)
        {
            _tprintf(_T("send() failed, thread for %d ends.\n"), sockfd);
            break;
        }    
    }

    closesocket(sockfd);
    _tprintf( _T("socket %d closed at %s.\n"), sockfd, Now().data());

    return 0;
}


SOCKET  create_listen_socket(const TCHAR* host, const TCHAR* port)
{
    ADDRINFOT* aiList = NULL;
    ADDRINFOT hints = {};
    hints.ai_family = AF_INET; // TCP v4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // used for bind()
    int error = GetAddrInfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        _tprintf(_T("getaddrinfo() failed, %s, %s.\n"), host, port);
        return INVALID_SOCKET;
    }

    // loop through the info list, listen the first we can
    SOCKET sockfd = INVALID_SOCKET;
    for (ADDRINFOT* pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            _tprintf(_T("socket() failed.\n"));			
            continue;
        }
        error = bind(sockfd, pinfo->ai_addr, pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            _tprintf(_T("bind() failed, addr: %s, len: %d.\n"), pinfo->ai_addr, pinfo->ai_addrlen);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        error = listen(sockfd, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            _tprintf(_T("listen() failed.\n"));
            closesocket(sockfd);
            continue;
        }        
        break; // succeed, break the loop
    }

    FreeAddrInfo(aiList);
    _tprintf(_T("server listen at %s:%s, %s.\n"), host, port, Now().data());

    return sockfd;
}

void print_usage()
{
    _tprintf(_T("Usage: $program $host $port"));
}
