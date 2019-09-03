// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "AsyncSelectPoller.h"
#include <assert.h>
#include <algorithm>
#include "Common/Error.h"
#include "Common/Logging.h"
#include "Common/StringUtil.h"
#include "Mask.h"


// define socket message
#define WM_SOCKET   WM_USER + 0x0F


AsyncSelectPoller::AsyncSelectPoller()
    : hwnd_(NULL), has_retired_(false)
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
	if (fds_.size() >= FD_SETSIZE)
	{
		return -1;
	}

	FdEntry entry = {};
    entry.fd = fd;
    entry.sink = event;
	fds_.push_back(entry);
    return 0;
}

// The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
// and cancels any previous WSAAsyncSelect or WSAEventSelect for the same socket.
void AsyncSelectPoller::RemoveFd(SOCKET fd)
{
	MarkRetired(fd);
    int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, 0);
    if (r == SOCKET_ERROR)
    {
        LOG(ERROR) << StringPrintf("WSAsyncSelect: %s", LAST_ERROR_MSG);
    }
}

void AsyncSelectPoller::SetPollIn(SOCKET fd)
{
	FdEntry* entry = FindEntry(fd);
	if (entry != NULL)
    {
        long lEvent = FD_READ | FD_ACCEPT | FD_CLOSE;
        entry->event |= lEvent;
        entry->mask |= MASK_READABLE;
        int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, entry->event);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("SetPollIn£º WSAsyncSelect %s", LAST_ERROR_MSG);
        }
    }
}

void AsyncSelectPoller::ResetPollIn(SOCKET fd)
{
	FdEntry* entry = FindEntry(fd);
	if (entry != NULL)
	{
        long lEvent = FD_READ | FD_ACCEPT | FD_CLOSE;
        entry->event &= ~lEvent;
		entry->mask &= ~MASK_READABLE;
        int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, entry->event);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("ResetPollIn£º WSAsyncSelect %s", LAST_ERROR_MSG);
        }
	}
}

void AsyncSelectPoller::SetPollOut(SOCKET fd)
{
	FdEntry* entry = FindEntry(fd);
	if (entry != NULL)
	{
        long lEvent = FD_WRITE | FD_CONNECT | FD_CLOSE;
        entry->event |= lEvent;
        entry->mask |= MASK_WRITABLE;
        int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, entry->event);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("SetPollOut£º WSAsyncSelect %s", LAST_ERROR_MSG);
            return;
        }
	}
}

void AsyncSelectPoller::ResetPollOut(SOCKET fd)
{
	FdEntry* entry = FindEntry(fd);
	if (entry != NULL)
	{
        long lEvent = FD_WRITE | FD_CONNECT | FD_CLOSE;
        entry->event &= ~lEvent;
		entry->mask &= ~MASK_WRITABLE;
        int r = WSAAsyncSelect(fd, hwnd_, WM_SOCKET, entry->event);
        if (r == SOCKET_ERROR)
        {
            LOG(ERROR) << StringPrintf("ResetPollOut£º WSAsyncSelect %s", LAST_ERROR_MSG);
        }
	}
}

// The FD_WRITE event is handled slightly differently.
// an application can assume that sends are possible starting from the first FD_WRITE message,
// and lasting until a send returns WSAEWOULDBLOCK.
void AsyncSelectPoller::HandleEvent(SOCKET fd, int ev, int ec)
{
    last_err_ = ec;
	FdEntry* entry = FindEntry(fd);
	if (entry == NULL)
    {
        fprintf(stderr, "socket %d entry not found, event %x\n", fd, ev);
        return;
    }
    switch(ev)
    {
    case FD_READ:
    case FD_ACCEPT:
    case FD_CLOSE:
        if (entry->mask | MASK_READABLE)
        {
            entry->sink->OnReadable();
        }
        break;

    case FD_WRITE:
    case FD_CONNECT:
        if (entry->mask | MASK_WRITABLE)
        {
            entry->sink->OnWritable();
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

AsyncSelectPoller::FdEntry* AsyncSelectPoller::FindEntry(SOCKET fd)
{
	for (size_t i = 0; i < fds_.size(); i++)
	{
		if (fds_[i].fd == fd)
		{
			return &fds_[i];
		}
	}
	return NULL;
}

void AsyncSelectPoller::MarkRetired(SOCKET fd)
{
	FdEntry* entry = FindEntry(fd);
	if (entry != NULL)
	{
		entry->fd = INVALID_SOCKET;
		entry->mask = 0;
		entry->sink = NULL;
	}
	has_retired_ = true;
}

void AsyncSelectPoller::RemoveRetired()
{
	if (has_retired_)
	{
		auto iter = std::remove_if(fds_.begin(), fds_.end(), [](const FdEntry& entry)
		{
			return entry.fd == INVALID_SOCKET;
		});
		fds_.erase(iter, fds_.end());
		has_retired_ = false;
	}
}
