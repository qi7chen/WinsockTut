/**
 *  @file   select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by select()
 *				
 */

#include "../common/utility.h"
#include <WS2tcpip.h>
#include <assert.h>
#include <stdio.h>
#include <set>

#pragma warning(disable:4127)

// total connections
namespace {
    std::set<SOCKET>    g_total_sockets;
}

 
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


bool on_accept(SOCKET sockfd)
{
    // evil 64 limit
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

    // set to non-blocking mode
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


void on_close(SOCKET sockfd)
{
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now().data());
    closesocket(sockfd);
}

// create acceptor
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

        // set to non-blocking mode
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


bool select_loop(SOCKET acceptor)
{
    fd_set readset = {}; 

    FD_SET(acceptor, &readset);
    
    for (std::set<SOCKET>::const_iterator iter = g_total_sockets.begin();
        iter != g_total_sockets.end(); ++iter)
    {
        FD_SET(*iter, &readset);
    }

    // 50 ms timeout
    timeval timeout = {0, 500*1000};
    int nready = select(0, &readset, NULL, NULL, &timeout);
    if (nready == SOCKET_ERROR)
    {
        fprintf(stderr, ("select() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    if (nready == 0) // timed out
    {            
        return true;
    }

    // check connection for read/write
    std::set<SOCKET>::iterator iter = g_total_sockets.begin();
    while (iter != g_total_sockets.end())
    {
        SOCKET fd = *iter;
        if (FD_ISSET(fd, &readset))
        {
            if (!on_recv(fd))
            {
                on_close(fd); 
                iter = g_total_sockets.erase(iter);
                continue;
            }
        }
        ++iter;
    }

    // have new connection?
    if (FD_ISSET(acceptor, &readset))
    {
        on_accept(acceptor);
    }

    return true;
}

// main entry
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
        ;

    closesocket(sockfd); // close the listen socket
    return 0;
}

