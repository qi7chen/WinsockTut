#include <stdio.h>
#include <stdlib.h>
#include "complete_routine.h"
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
    acceptor = create_acceptor(host, atoi(port));
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    while (event_loop(acceptor))
        ;

    WSACleanup();
    return 0;
}

