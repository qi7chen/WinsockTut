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
    int r = 0;
    if (masks_[fd] == 0)
    {
        long lEvent = 0;
        lEvent |= FD_ACCEPT;
        lEvent |= FD_READ;
        lEvent |= FD_CLOSE;
        lEvent |= FD_CONNECT;
        lEvent |= FD_WRITE;

        //The WSAAsyncSelect function automatically sets socket s to nonblocking mode
        r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, lEvent);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
        }
    }
    masks_[fd] |= mask;
    return r;
}

void AsyncSelectPoller::DelFd(SOCKET fd, int mask)
{
    masks_[fd] &= ~mask;
    if (masks_[fd] == 0)
    {
        int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, 0);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
        }
    }
}

void AsyncSelectPoller::HandleEvent(EventLoop* loop, SOCKET fd, int ev, int ec)
{
    if (ec > 0)
    {
        loop->AddFiredEvent(fd, EV_READABLE, ec);
        return;
    }
    if ((ev & FD_READ) || (ev & FD_ACCEPT) || (ev & FD_CLOSE))
    {
        if (masks_[fd] & EV_READABLE)
        {
            loop->AddFiredEvent(fd, EV_READABLE, ec);
        }
    }
    if ((ev & FD_WRITE) || (ev & FD_CONNECT))
    {
        if (masks_[fd] & EV_WRITABLE)
        {
            loop->AddFiredEvent(fd, EV_WRITABLE, ec);
        }
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
