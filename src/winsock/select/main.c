#include "select.h"
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

    CHECK(select_init());

    acceptor = create_acceptor(host, port);
    if (acceptor == INVALID_SOCKET)
    {
        return 1;
    }

    while (select_loop(acceptor))
    {
    }

    select_release();


    return 0;
}

