// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncSelect.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "Common/Util.h"
#include "EventLoop.h"

// define socket message
#define WM_SOCKET   WM_USER + 0xBF

AsyncSelectPoller::AsyncSelectPoller()
{
    hwnd_ = NULL;
    CreateHidenWindow();
}

AsyncSelectPoller::~AsyncSelectPoller()
{
    CloseHandle(hwnd_);
    hwnd_ = NULL;
}

void AsyncSelectPoller::CreateHidenWindow()
{
    const wchar_t* szTitle = L"async-select";
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
    HWND hWnd = CreateWindowW(L"button", szTitle, WS_POPUP, CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    CHECK(hWnd != NULL) << LAST_ERROR_MSG;
    ShowWindow(hWnd, SW_HIDE);
    hwnd_ = hWnd;
}

int AsyncSelectPoller::AddFd(SOCKET fd, int mask)
{
    long lEvent = FD_CLOSE;
    if (mask & EV_READABLE)
    {
        lEvent |= FD_ACCEPT;
        lEvent |= FD_READ;
    }
    if (mask & EV_WRITABLE)
    {
        lEvent |= FD_CONNECT;
        lEvent |= FD_WRITE;
    }
    //The WSAAsyncSelect function automatically sets socket s to nonblocking mode
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, lEvent);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
    return r;
}

void AsyncSelectPoller::DelFd(SOCKET fd, int mask)
{
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, 0);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
}

void AsyncSelectPoller::HandleEvent(EventLoop* loop, SOCKET fd, int event, int ec)
{
    if (ec > 0)
    {
        loop->AddFiredEvent(fd, EV_READABLE, ec);
        return;
    }
    switch (event)
    {
    case FD_READ:
    case FD_ACCEPT:
    case FD_CLOSE:
        loop->AddFiredEvent(fd, EV_READABLE, ec);
        break;
    case FD_WRITE:
    case FD_CONNECT:
        loop->AddFiredEvent(fd, EV_WRITABLE, ec);
        break;
    default:
        break;
    }
}

int AsyncSelectPoller::Poll(EventLoop* loop, int timeout)
{
    int count = 0;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_SOCKET)
        {
            SOCKET fd = msg.wParam;
            int event = WSAGETSELECTEVENT(msg.lParam);
            int error = WSAGETSELECTERROR(msg.lParam);
            HandleEvent(loop, fd, event, error);
            count++;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return count;
}
