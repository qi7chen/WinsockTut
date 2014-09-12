#include <stdio.h>
#include <process.h>
#include "socket.h"
#include "common/utility.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")


int main(int argc, const char* argv[])
{    
    SOCKET acceptor;
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;

    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    CHECK(socket_init());

    acceptor = create_acceptor(host, port);
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    while (socket_loop(acceptor))
        ;

    socket_release();

    return 0;
}