#include <stdio.h>
#include <stdlib.h>
#include "async_event.h"
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
    SOCKET socketListen = create_listen_socket(host, atoi(port));
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

