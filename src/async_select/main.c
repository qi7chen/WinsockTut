/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "appdef.h"
#include <assert.h>
#include "common/utility.h"


/* create a hiden window */
HWND IntiInstance(HINSTANCE hInstance)
{
    const char* szTitle = "AsyncSelect";
    HWND hWnd = CreateWindowA("button", szTitle, WS_POPUP, CW_USEDEFAULT, 0,
                        CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    CHECK(hWnd != NULL);
    ShowWindow(hWnd, SW_HIDE);
    return hWnd;
}


int main(int argc, const char* argv[])
{
    HWND hWnd;
    MSG msg;
    WSADATA data;
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    if (argc > 2)
    {
        host = argv[1];
        port = argv[2];
    }

    hWnd = IntiInstance((HINSTANCE)GetModuleHandle(NULL));
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    CHECK(InitEchoServer(hWnd, host, port) == 0);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_SOCKET)
        {
            SOCKET fd = msg.wParam;
            int event = WSAGETSELECTEVENT(msg.lParam);
            int error = WSAGETSELECTERROR(msg.lParam);
            HandleNetEvents(hWnd, fd, event, error);
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    WSACleanup();
    CloseServer();
    return 0;
}
