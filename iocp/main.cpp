
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "iocp.h"




int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("%s $host $port"), argv[0]);
        exit(1);
    }

    const TCHAR* host = argv[1];
    short port = (short)_ttoi(argv[2]);

    iocp_server server;
    server.start(host, port);

    return 0;
}

