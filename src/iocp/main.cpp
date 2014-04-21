#include <stdlib.h>
#include <stdio.h>
#include "iocp.h"
#include "../common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("IOCP [host] [port]\n"));
        return 1;
    }

    WinsockInit init;
    IOCPServer server;
    server.start(argv[1], atoi(argv[2]));

    return 0;
}

