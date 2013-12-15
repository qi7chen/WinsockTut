/**
 *  @file   main.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用完成例程模型实现的简单Echo Server
 *			
 */

#include <stdio.h>
#include "complete_routine.h"


SOCKET  create_listen_socket(const char* host, int port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() [%s:%d]failed, %s"), host, port, LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    error = listen(sockfd, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    // 非阻塞模式
    ULONG nonblock = 1;
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    fprintf(stderr, ("server start listen [%s:%d] at %s.\n"), host, port, Now().data());
    return sockfd;
}



int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: CompleteRoutine [host] [port]\n"), argv[0]);
        return 1;
    }

    WinsockInit init;
    SOCKET sockfd = create_listen_socket(argv[1], atoi(argv[2]));
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        sockaddr_in addr = {};
        int addrlen = sizeof(addr);
        SOCKET socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
        if (socknew != INVALID_SOCKET)
        {
            socket_data* data = alloc_data(socknew);
            fprintf(stderr, ("socket %d accepted at %s.\n"), socknew, Now().data());
            if (data)
            {
                post_recv_request(data);
            }
        }
        else
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {   
                fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
                break;
            }
            else
            {
                ::SleepEx(50, TRUE); // 可提醒I/O
            }
        }        
    }

    return 0;
}

