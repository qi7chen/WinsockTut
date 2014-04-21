#include <stdio.h>
#include <stdlib.h>
#include "overlap.h"
#include "../common/utility.h"


#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


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
