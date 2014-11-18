#include <stdio.h>
#include "server.h"


int main(int argc, const char* argv[])
{
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    if (server_init(host, (short)atoi(port)))
    {
        while (server_run())
            ;
    }
    server_destroy();

    return 0;
}

