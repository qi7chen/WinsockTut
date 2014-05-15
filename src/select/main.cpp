#include <stdio.h>
#include <stdlib.h>
#include "select.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


// main entry
int main(int argc, const char* argv[])
{
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    WinsockInit init;
    SOCKET socketListen = create_listen_socket(host, port);
    if (socketListen == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        select_loop(socketListen);
    }
}

