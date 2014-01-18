#include <stdio.h>
#include <stdlib.h>
#include "overlap.h"


// Create acceptor
SOCKET create_listen_socket(const char* host, int port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    int error = bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
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

    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now().data());
    return sockfd;
}


// main entry
int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: Overlapped [host] [port]\n"));
        return 1;
    }

    WinsockInit init;
    SOCKET socketListen = create_listen_socket(argv[1], atoi(argv[2]));
    if (socketListen == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        SOCKET sockAccept = accept(socketListen, 0, 0);
        if (sockAccept != INVALID_SOCKET)
        {
            if (!on_accept(sockAccept))
            {
                closesocket(sockAccept);
            }
        }
        else
        {
            if (GetLastError() == WSAEWOULDBLOCK)
            {
                if (event_loop())
                {
                    continue;
                }
            }
            fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
            break;
        }
    }
}
