/**
 *  @file   main.cpp
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 *  @brief:  使用I/O完成端口单线程事件循环实现的多个客户端连接测试程序
 */

#include <tchar.h>
#include <stdlib.h>
#include "clients.h"

#pragma comment(lib, "ws2_32")



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