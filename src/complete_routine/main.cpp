#include <stdio.h>
#include "complete_routine.h"


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
    SOCKET sockfd = create_listen_socket(host, atoi(port));
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        sockaddr_in addr = {};
        int addrlen = sizeof(addr);
        SOCKET socknew = accept(sockfd, (sockaddr*)&addr, &addrlen);
        if (socknew != INVALID_SOCKET)
        {
            socket_data* data = alloc_data(socknew);
            fprintf(stderr, ("socket %d accepted at %s.\n"), socknew, Now().data());
            if (data)
            {
                post_recv_request(data);
            }
        }
        else
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {   
                fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
                break;
            }
            else
            {
                ::SleepEx(50, TRUE); // make thread alertable
            }
        }        
    }

    return 0;
}

