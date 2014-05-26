#include <stdio.h>
#include <process.h>
#include "socket.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


int main(int argc, const char* argv[])
{
    WSADATA data;
    SOCKET acceptor;
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }
    
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);

    acceptor = create_acceptor(host, port);
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SOCKET sockfd = accept(acceptor, (struct sockaddr*)&addr, &len);
        if (sockfd == INVALID_SOCKET)
        {
            fprintf(stderr, "accept() failed, %s.\n", LAST_ERROR_MSG);
            break;
        }

        on_accept(sockfd);       
    }

    WSACleanup();
    return 0;
}