/**
 *  @brief:  A simple socket client, use fundamental api
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */

#include <tchar.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../../common/utility.h"




int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        exit(1);
    }

    const TCHAR* host = argv[1];
    const TCHAR* port = argv[2];

    ADDRINFOT* aiList = NULL;
    ADDRINFOT hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME;
    int error = GetAddrInfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        LOG_PRINT(_T("getaddrinfo() failed, %s, %s"), host, port);
        return 1;
    }

    // loop through the info list, connect the first we can
    SOCKET socket_connect = INVALID_SOCKET;
    for (ADDRINFOT* pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        socket_connect = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (socket_connect == INVALID_SOCKET)
        {
            LOG_PRINT(_T("socket() failed"));
            continue;
        }

        error = connect(socket_connect, pinfo->ai_addr, pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            LOG_PRINT(_T("connect() failed, addr: %s, len: %d"), pinfo->ai_addr, pinfo->ai_addrlen);
            closesocket(socket_connect);
            socket_connect = INVALID_SOCKET;
            continue;
        }

        _tprintf(_T("connect %s:%s OK\n"), host, port);
        break;
    }

    // check if we find a valid service provider
    if (socket_connect == INVALID_SOCKET)
    {
        return 1;
    }

    FreeAddrInfo(aiList);

    TCHAR databuf[BUFSIZ];
    const TCHAR* test_msg = _T("A quick fox jumps over the lazy dog");

    for (;;)
    {
        int count_num = 0;
        for (;;)
        {
            int msg_len = _stprintf_s(databuf, BUFSIZ, _T("%s, %d"), test_msg, ++count_num);
            int bytes_send = (msg_len + 1) * sizeof(TCHAR);
            bytes_send = send(socket_connect, (const char*)databuf, bytes_send, 0);
            if (bytes_send == SOCKET_ERROR)
            {
                LOG_PRINT(_T("send() failed"));
                closesocket(socket_connect);
                break;
            }

            int bytes_read = recv(socket_connect, (char*)databuf, BUFSIZ, 0);
            if (bytes_read == SOCKET_ERROR)
            {
                LOG_PRINT(_T("recv() failed"));
                closesocket(socket_connect);
                break;
            }
            else if (bytes_read == 0)
            {
                closesocket(socket_connect);
                break;
            }

            ::Sleep(count_num * 10); // to have a rest
        }    
    }
}





