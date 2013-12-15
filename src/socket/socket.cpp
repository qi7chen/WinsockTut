/**
 *  @file   socket.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用 BSD socket API实现的简单Echo Server
 *			为每个客户端连接创建一个线程
 */


#include "../common/utility.h"
#include <stdio.h>



// 处理单个连接
unsigned CALLBACK handle_client(void* param)
{
    SOCKET sockfd = (SOCKET)param;
    char databuf[kDefaultBufferSize];
    for (;;)
    {
        int bytes_read = recv(sockfd, databuf, kDefaultBufferSize, 0);
        if (bytes_read == SOCKET_ERROR)
        {
            fprintf(stderr, ("socket %d recv() failed, %s"), sockfd, LAST_ERROR_MSG);
            break;
        }
        else if (bytes_read == 0) // 连接已经关闭
        {
            break;
        }

        // 将收到的消息发送回客户端
        int bytes_send = send(sockfd, databuf, bytes_read, 0);
        if (bytes_send == SOCKET_ERROR)
        {
            fprintf(stderr, ("socket %d send() failed, %s"), sockfd, LAST_ERROR_MSG);
            break;
        }    
    }

    closesocket(sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now().data());

    return 0;
}


// 创建接收套接字
SOCKET  create_listen_socket(const char* host, const char* port)
{    
    addrinfo* aiList = NULL;
    addrinfo hints = {};
    hints.ai_family = AF_INET;          // TCP v4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int error = getaddrinfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        fprintf(stderr, ("getaddrinfo() failed, %s:%s, %s.\n"), host, port, gai_strerror(error));
        return INVALID_SOCKET;
    }
    
    SOCKET sockfd = INVALID_SOCKET;
    for (ADDRINFOT* pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
            continue;
        }
        int error = bind(sockfd, pinfo->ai_addr, pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, ("bind() failed, addr: %s, len: %d, %s"), 
                pinfo->ai_addr, pinfo->ai_addrlen, LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        error = listen(sockfd, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
            closesocket(sockfd);
            continue;
        }        
        break;
    }

    freeaddrinfo(aiList);
    fprintf(stdout, ("server listen at %s:%s, %s.\n"), host, port, Now().data());

    return sockfd;
}




int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: socket [host] [port]\n");
        return 1;
    }

    WinsockInit init;

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
            fprintf(stderr, ("accept() failed, %s.\n"), LAST_ERROR_MSG);
            break;
        }

        fprintf(stdout, ("socket %d accepted at %s.\n"), sock_accept, Now().data());

        // one thread per connection
        StartThread(handle_client, sock_accept);   
    }

    return 0;
}

