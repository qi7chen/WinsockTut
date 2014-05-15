#include <stdio.h>
#include <stdlib.h>
#include "async_event.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


// main entry
int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: socket [host] [port].\n");
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
        SOCKET socknew = accept(socketListen, NULL, NULL);
        if (socknew != SOCKET_ERROR)
        {
            on_accept(socknew);
        }
        else
        {
            event_loop();
        }
    }
}

