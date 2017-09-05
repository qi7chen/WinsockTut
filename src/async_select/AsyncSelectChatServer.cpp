/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "AsyncSelectChatServer.h"
#include <stdio.h>
#include <WS2tcpip.h>
#include "common/Utils.h"

#define WM_SOCKET       WM_USER + 0xF0


AsyncSelectChatServer::AsyncSelectChatServer()
{
    acceptor_ = INVALID_SOCKET;
    hWnd_ = NULL;
}

AsyncSelectChatServer::~AsyncSelectChatServer()
{
    closesocket(acceptor_);
    CloseHandle(hWnd_);
    acceptor_ = INVALID_SOCKET;
    hWnd_ = NULL;
}

bool AsyncSelectChatServer::Init(const char* host, const char* port)
{
    HWND hWnd = CreateHidenWindow();
    if (hWnd == NULL)
    {
        return false;
    }
    SOCKET fd = CreateTCPAcceptor(host, port, false);

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms741540(v=vs.85).aspx
    //
    //The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
    //regardless of the value of lEvent.        
    if (WSAAsyncSelect(fd, hWnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(fd);
        CloseHandle(hWnd);
        return false;
    }
    acceptor_ = fd;
    hWnd_ = hWnd;
    return true;
}

HWND AsyncSelectChatServer::CreateHidenWindow()
{
    const char* szTitle = "async-select";
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
    HWND hWnd = CreateWindowA("button", szTitle, WS_POPUP, CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (hWnd == NULL)
    {
        fprintf(stderr, "CreateWindow(): %s\n", LAST_ERROR_MSG);
        return NULL;
    }
    ShowWindow(hWnd, SW_HIDE);
    return hWnd;
}

int AsyncSelectChatServer::Run()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_SOCKET)
        {
            SOCKET fd = msg.wParam;
            int event = WSAGETSELECTEVENT(msg.lParam);
            int error = WSAGETSELECTERROR(msg.lParam);
            HandleSocketEvent(fd, event, error);
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

void AsyncSelectChatServer::HandleSocketEvent(SOCKET fd, int event, int error)
{
    if (error > 0)
    {
        return;
    }
    switch (event)
    {
    case FD_ACCEPT:
        HandleAccept(fd);
    }
}

void AsyncSelectChatServer::HandleAccept(SOCKET acceptor)
{
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET fd = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (fd == INVALID_SOCKET)
    {
        fprintf(stderr, ("accpet() failed, %s"), LAST_ERROR_MSG);
        return;
    }
    int err = WSAAsyncSelect(fd, hWnd_, WM_SOCKET, FD_WRITE | FD_READ | FD_CLOSE);
    if (err == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(fd);
        return;
    }
    char addrbuf[64] = {};
    inet_ntop(addr.sin_family, &addr.sin_addr, addrbuf, 64);
    fprintf(stdout, ("socket %d from %s accepted at %s.\n"), fd, addrbuf, Now());
}
