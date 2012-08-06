/**
 *  @brief:  A simple socket client, use I/O completion port and ConnectEx()
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include <tchar.h>
#include <stdlib.h>
#include "clients.h"

int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 4)
    {
        _tprintf(_T("Usage: %s $host $port $counts"), argv[0]);
        return 1;
    }

    const TCHAR* host = argv[1];
    short port = (short)_ttoi(argv[2]);
    int count = _ttoi(argv[3]);

    clients clients_mgr;
    clients_mgr.start(host, port, count);

    return 0;
}