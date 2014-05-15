#include <stdlib.h>
#include <stdio.h>
#include "iocp.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


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
    IOCPServer server;
    server.start(host, atoi(port));

    return 0;
}

