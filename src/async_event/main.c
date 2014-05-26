#include <stdio.h>
#include <stdlib.h>
#include "async_event.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


// main entry
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
    async_event_init();
    acceptor = create_acceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        SOCKET sockfd = accept(acceptor, NULL, NULL);
        if (sockfd != SOCKET_ERROR)
        {
            if (!on_accept(sockfd))
                break;
        }
        else
        {
            if (!event_loop())
                break;
        }
    }
    async_event_release();
}

