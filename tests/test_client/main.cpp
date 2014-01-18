#include <stdlib.h>
#include "clients.h"


int main(int argc, const char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, ("Usage: TestClient [host] [port] [count]\n"));
        return 1;
    }

    WinsockInit init;
    const char* host = argv[1];
    short port = (short)atoi(argv[2]);
    int count = atoi(argv[3]);

    clients clients_mgr;
    clients_mgr.start(host, port, count);

    return 0;
}