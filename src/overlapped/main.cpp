/**
 *  @brief:  A simple echo server, use winsock Overlapped I/O model
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include "stdafx.h"
#include "../../Common/Common.h"
#include "../../Common/Mutex.h"


enum {MODEL_SIMPLE = 1, MODEL_THREADED, MODEL_COROUTINE};


int OverlapWithEvent(const char* host, short port);
int OverlapWithCoRoutine(const char* szhost, short port);




int main(int argc, char* argv[])
{
    short port = 3245;
    const char* szHost = "127.0.0.1";
    int model = 0;

    if (argc < 4)
    {
        wprintf(_W("Usage: <program>  [@host] [@port] [@model]\n"));
        return 0;
    }
    switch(argc)
    {
    case 4:
        model = atoi(argv[3]);

    case 3:
        port = (short)atoi(argv[2]);

    case 2:
        szHost = argv[1];
    }

    switch(model)
    {
    case MODEL_SIMPLE:
        return OverlapWithEvent(szHost, port);
        break;

    case MODEL_COROUTINE:
        OverlapWithCoRoutine(szHost, port);
        break;
    }
}

