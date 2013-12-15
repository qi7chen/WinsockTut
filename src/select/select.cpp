/**
 *  @file   select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用select模型实现的简单Echo Server
 *			
 */

#include "../common/utility.h"
#include <assert.h>
#include <set>



#pragma warning(disable:4127)

// 所有的客户端套接字
std::set<SOCKET>    g_total_sockets;


// 处理接收数据
bool on_recv(SOCKET sockfd)
{
    char buf[kDefaultBufferSize];
    int bytes = recv(sockfd, buf, kDefaultBufferSize, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, ("recv() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    if (bytes == 0)
    {
        return false;
    }

    bytes = send(sockfd, buf, bytes, 0);
    if (bytes == SOCKET_ERROR)
    {
        fprintf(stderr, ("send() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    return true;
}

// 处理客户端连入
bool on_accept(SOCKET sockfd)
{
    // 达到上限
    if (g_total_sockets.size() == FD_SETSIZE - 1)
    {
        fprintf(stderr, ("got the 64 limit.\n"));
        return false;
    }

    sockaddr_in addr = {};
    int addrlen = sizeof(addr);
    int socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
    if (socknew == INVALID_SOCKET)
    {
        fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    // 设置为非阻塞模式
    ULONG nonblock = 1;
    if (ioctlsocket(socknew, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(socknew);
        return false;
    }

    fprintf(stdout, ("socket %d accepted at %s.\n"), socknew, Now().data());

    g_total_sockets.insert(socknew);
    return true;
}

// 处理客户端连接关闭
void on_close(SOCKET sockfd)
{
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now().data());
    closesocket(sockfd);
}

// 创建监听套接字
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
        // 需要将套接字设置为非阻塞模式
        ULONG nonblock = 1;
        if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
        {
            fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
            closesocket(sockfd);
            return INVALID_SOCKET;
        }
        break;
    }

    freeaddrinfo(aiList);
    fprintf(stdout, ("server listen at %s:%s, %s.\n"), host, port, Now().data());
    return sockfd;
}


bool select_loop(SOCKET sockfd)
{
    fd_set readset = {}; 

    // 添加监听套接字
    FD_SET(sockfd, &readset);

    // 添加客户端套接字 
    for (std::set<SOCKET>::const_iterator iter = g_total_sockets.begin();
        iter != g_total_sockets.end(); ++iter)
    {
        FD_SET(*iter, &readset);
    }

    // 50毫秒的超时
    timeval timeout = {0, 500*1000};
    int nready = select(0, &readset, NULL, NULL, &timeout);
    if (nready == SOCKET_ERROR)
    {
        fprintf(stderr, ("select() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    if (nready == 0) // 超时处理
    {            
        return true;
    }

    // 检查接收数据
    std::set<SOCKET>::iterator iter = g_total_sockets.begin();
    while (iter != g_total_sockets.end())
    {
        SOCKET fd = *iter;
        if (FD_ISSET(fd, &readset))
        {
            if (!on_recv(fd))
            {
                on_close(fd); // 失败后关闭连接
                iter = g_total_sockets.erase(iter);
                continue;
            }
        }
        ++iter;
    }

    // 是否有新连接
    if (FD_ISSET(sockfd, &readset))
    {
        on_accept(sockfd);
    }

    return true;
}

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: select [host] [port]\n"));
        return 1;
    }

    WinsockInit init;
    SOCKET sockfd = create_listen_socket(argv[1], argv[2]);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }
    
    while (select_loop(sockfd))
    {        
    }

    closesocket(sockfd); // close the listen socket
    return 0;
}

