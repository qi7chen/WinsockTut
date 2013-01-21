//  A simple echo server, use I/O Completion Port model
//  by ichenq@gmail.com at Oct 19, 2011


#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "iocp.h"

#pragma comment(lib, "ws2_32")


// initialize winsock dll
static global_init init;


// main entry
int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("%s $host $port"), argv[0]);
        return 1;
    }

    iocp_server server;
    server.start(argv[1], argv[2]);

    return 0;
}

