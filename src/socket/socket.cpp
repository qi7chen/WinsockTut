/**
*  @file   socket.cpp
*  @author ichenq@gmail.com
*  @date   Oct 19, 2011
*  @brief  A simple echo server, use fundamental socket api

*/



#include "../common/utility.h"
#include "../common/logging.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>
#include <memory>
#include <functional>





static SOCKET      create_listen_socket(const TCHAR* host, const TCHAR* port);
static unsigned    handle_client(unsigned sockfd);



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

    for (;;)
    {
        sockaddr_in addr = {};
        int len = sizeof(addr);
        SOCKET sock_accept = accept(sockfd, (sockaddr*)&addr, &len);
        if (sock_accept == INVALID_SOCKET)
        {
            LOG_PRINT(_T("listen() failed"));
            break;
        }
        try
        {            
            _tprintf(_T("%s, socket %d accepted.\n"), Now().data(), sock_accept);

            // one thread per connection
            start_thread(handle_client, sock_accept);
        }
        catch (std::bad_alloc&)
        {
            LOG_PRINT(_T("allocate thread object failed"));
        }        
    }

    return 0;
}


// run in client thread
unsigned    handle_client(unsigned sockfd)
{
    char databuf[BUFSIZ];
    for (;;)
    {
        int bytes_read = recv(sockfd, databuf, BUFSIZ, 0);
        if (bytes_read == SOCKET_ERROR)
        {
            LOG_PRINT(_T("recv() failed\n"));
            break;
        }
        else if (bytes_read == 0)
        {
            break;
        }

        // write back what we read
        int bytes_send = send(sockfd, databuf, bytes_read, 0);
        if (bytes_send == SOCKET_ERROR)
        {
            LOG_PRINT(_T("send() failed\n"));
            break;
        }    
    }

    closesocket(sockfd);
    _tprintf( _T("%s, socket %d closed.\n"), Now().data(), sockfd);

    return 0;
}


SOCKET  create_listen_socket(const TCHAR* host, const TCHAR* port)
{
    ADDRINFOT* aiList = NULL;
    ADDRINFOT hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME;
    int error = GetAddrInfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        LOG_PRINT(_T("getaddrinfo() failed, %s, %s"), host, port);
        return INVALID_SOCKET;
    }

    // loop through the info list, connect the first we can
    SOCKET sockfd = INVALID_SOCKET;
    for (ADDRINFOT* pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            LOG_PRINT(_T("socket() failed"));			
            continue;
        }
        error = bind(sockfd, pinfo->ai_addr, pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            LOG_PRINT(_T("bind() failed, addr: %s, len: %d"), pinfo->ai_addr, pinfo->ai_addrlen);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        error = listen(sockfd, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            LOG_PRINT(_T("listen() failed"));
            closesocket(sockfd);
            continue;
        }        
        break; // succeed, break the loop
    }

    FreeAddrInfo(aiList);
    _tprintf(_T("%s, server listen at %s:%s.\n"), Now().data(), host, port);

    return sockfd;
}
