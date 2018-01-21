// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncSelect.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "Common/Util.h"
#include "EventLoop.h"

// define socket message
#define WM_SOCKET   WM_USER + 0x0F

AsyncSelectPoller::AsyncSelectPoller()
    : hwnd_(NULL)
{
    CreateHidenWindow();
}

AsyncSelectPoller::~AsyncSelectPoller()
{
    CloseHandle(hwnd_);
    hwnd_ = NULL;
}

void AsyncSelectPoller::CreateHidenWindow()
{
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
    HWND hWnd = CreateWindowW(L"button", L"async-select", WS_POPUP, CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    CHECK(hWnd != NULL) << LAST_ERROR_MSG;
    ShowWindow(hWnd, SW_HIDE);
    hwnd_ = hWnd;
}

int AsyncSelectPoller::AddFd(SOCKET fd)
{
    long lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE;

    // The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
    // and cancels any previous WSAAsyncSelect or WSAEventSelect for the same socket.
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, lEvent);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
    return r;
}

void AsyncSelectPoller::DeleteFd(SOCKET fd)
{
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, 0);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
}

void AsyncSelectPoller::HandleEvent(EventLoop* loop, SOCKET fd, int ev, int ec)
{
    if (ec > 0)
    {
        loop->AddFiredEvent(fd, EV_READABLE, ec);
        return;
    }
    switch(ev)
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
    }
}

int AsyncSelectPoller::Poll(EventLoop* loop, int timeout)
{
    int count = 0;
    MSG msg;
    while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_SOCKET)
        {
            SOCKET fd = msg.wParam;
            int ev = WSAGETSELECTEVENT(msg.lParam);
            int ec = WSAGETSELECTERROR(msg.lParam);
            HandleEvent(loop, fd, ev, ec);
            count++;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    if (count == 0 && timeout > 0)
    {
        Sleep(timeout);
    }
    return count;
}
