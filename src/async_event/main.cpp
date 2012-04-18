/**
 *  @brief:  A simple echo server, use winsock Overlapped I/O model
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "async_event.h"


int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        exit(1);
    }

    const TCHAR* host = argv[1];
    short port = (short)_ttoi(argv[2]);

    async_event_server server;
    server.start(host, port);

    return 0;
}

