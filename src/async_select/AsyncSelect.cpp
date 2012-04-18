

#include "stdafx.h"
#include "../../Common/Common.h"
#include "CmnDef.h"


#pragma warning(disable: 4127)


static HWND    g_hwnd;



void    HandleSocketEvent(SOCKET fd, int nEvent, int nError)
{
    if (nError)
    {
        closesocket(fd);
        return ;
    }

    switch(nEvent)
    {
    case FD_ACCEPT:
        {
            SOCKET sockAccept = accept(fd, NULL, NULL);
            // automatically sets socket to nonblocking mode
            if (WSAAsyncSelect(sockAccept, g_hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE) == SOCKET_ERROR)
            {
                PRINT2LOG(_W("WSAAsyncSelect() failed"), LAST_ERR_MSG);
            }
        }
        break;

    case FD_READ:
        {
            TCHAR recvbuf[BUFSIZ];
            int bytes = recv(fd, (char*)recvbuf, sizeof(TCHAR)*BUFSIZ, 0);
            if (bytes == SOCKET_ERROR || bytes == 0)
            {
                HandleSocketEvent(fd, FD_CLOSE, 0);
            }
            else
            {
                //MessageBox(NULL, (TCHAR*)recvbuf, _T("Message"), MB_OK);
                SendMessage(g_hwnd, WM_TEXT_UPDATE, (WPARAM)recvbuf, (LPARAM)bytes);
                send(fd, recvbuf, bytes, 0);
            }
        }
        break;

    case FD_WRITE:
        // do nothing
        break;

    case FD_CLOSE:
        {
            closesocket(fd);
        }
        break;

    default:
        assert(!_T("invalid event type"));
        break;
    }
}


BOOL    InitNetwork(HWND hWnd, const char* szHost, short port)
{
    g_hwnd = hWnd;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(szHost);

    SOCKET sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockListen == INVALID_SOCKET)
    {
        PRINT2LOG(_W("socket() failed"), LAST_ERR_MSG);
        return 0;
    }

    int status = bind(sockListen, (sockaddr*)&addr, sizeof(addr));
    if (status == SOCKET_ERROR)
    {
        PRINT2LOG(_W("bind() failed"), LAST_ERR_MSG);
        closesocket(sockListen);
        return 0;
    }

    status = listen(sockListen, SOMAXCONN);
    if (status == SOCKET_ERROR)
    {
        PRINT2LOG(_W("listen() failed"), LAST_ERR_MSG);
        closesocket(sockListen);
        return 0;
    }   

    if (WSAAsyncSelect(sockListen, hWnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        return FALSE;
    }

    return TRUE;
}

