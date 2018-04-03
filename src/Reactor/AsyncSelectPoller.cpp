// Copyright (C) 2012-2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncSelectPoller.h"
#include <assert.h>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringPrintf.h"
#include "Mask.h"


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

int AsyncSelectPoller::AddFd(SOCKET fd, IPollEvent* event)
{
    assert(event != nullptr);

    long lEvent = FD_READ | FD_WRITE | FD_CONNECT;

    // The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
    // and cancels any previous WSAAsyncSelect or WSAEventSelect for the same socket.
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, lEvent);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
    
    FdEntry entry;
    entry.mask = 0;
    entry.sink = event;
    fds_[fd] = entry;

    return r;
}

void AsyncSelectPoller::RemoveFd(SOCKET fd)
{
    fds_.erase(fd);
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, 0);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
}

void AsyncSelectPoller::SetPollIn(SOCKET fd)
{
    auto iter = fds_.find(fd);
    if (iter != fds_.end())
    {
        iter->second.mask |= MASK_READABLE;
    }
}

void AsyncSelectPoller::ResetPollIn(SOCKET fd)
{
    auto iter = fds_.find(fd);
    if (iter != fds_.end())
    {
        iter->second.mask &= ~MASK_READABLE;
    }
}

void AsyncSelectPoller::SetPollOut(SOCKET fd)
{
    auto iter = fds_.find(fd);
    if (iter != fds_.end())
    {
        iter->second.mask |= MASK_WRITABLE;
    }
}

void AsyncSelectPoller::ResetPollOut(SOCKET fd)
{
    auto iter = fds_.find(fd);
    if (iter != fds_.end())
    {
        iter->second.mask &= ~MASK_WRITABLE;
    }
}

// The FD_WRITE event is handled slightly differently.
// an application can assume that sends are possible starting from the first FD_WRITE message,
// and lasting until a send returns WSAEWOULDBLOCK.
void AsyncSelectPoller::HandleEvent(SOCKET fd, int ev, int ec)
{
    last_err_ = ec;
    auto iter = fds_.find(fd);
    if (iter == fds_.end())
    {
        fprintf(stderr, "socket %d not found in registration\n", fd);
        return;
    }
    FdEntry& entry = iter->second;
    switch(ev)
    {
    case FD_READ:
    case FD_ACCEPT:
    case FD_CLOSE:
        if (entry.mask | MASK_READABLE)
        {
            entry.sink->OnReadable();
        }
        break;

    case FD_WRITE:
    case FD_CONNECT:
        if (entry.mask | MASK_WRITABLE)
        {
            entry.sink->OnWritable();
        }
        break;
    }
}

int AsyncSelectPoller::Poll(int timeout)
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
            HandleEvent(fd, ev, ec);
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
        UpdateTimer();
        Sleep(timeout);
    }
    return count;
}
