#include <stdio.h>
#include "complete_routine.h"


// Create acceptor
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

    // set to non-blocking mode
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


// main entry
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
                ::SleepEx(50, TRUE); // make thread alertable
            }
        }        
    }

    return 0;
}

