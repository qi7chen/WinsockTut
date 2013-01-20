#include "worker.h"
#include <process.h>
#include <functional>
#include <algorithm>


unsigned CALLBACK NativeThreadFunc(void* param)
{
    // force windows initialize a message queue for this thread
    PeekMessage(NULL, NULL, 0, 0, PM_NOREMOVE); 
    worker* pWorker = (worker*)param;
    if (pWorker)
    {
        pWorker->main_loop();
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////

worker::worker()
    : count_(0), my_thread_(0)
{
    reset();
}

worker::~worker()
{
    reset();
}

void worker::start()
{
    _beginthreadex(NULL, 0, NativeThreadFunc, this, 0, &my_thread_);
}


bool worker::is_full()
{
    return WSA_MAXIMUM_WAIT_EVENTS == InterlockedCompareExchange((long*)&count_, 
        count_, WSA_MAXIMUM_WAIT_EVENTS);
}

//////////////////////////////////////////////////////////////////////////

void worker::main_loop()
{
    for (;;)
    {
        if (count_ == 0)
        {
            handle_messages();
            continue;
        }

        // 50 ms time out
        size_t nready = WSAWaitForMultipleEvents(count_, eventlist_, 
            TRUE, 50, FALSE);
        if (nready == WSA_WAIT_FAILED)
        {
            _tprintf(_T("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
            break;
        }
        else if (nready == WSA_WAIT_TIMEOUT)
        {
            handle_messages();
        }
        else if (nready == WSA_WAIT_IO_COMPLETION)
        {
        }
        else
        {
            size_t index = WSA_WAIT_EVENT_0 + nready;
            WSAEVENT hEvent = eventlist_[index];
            SOCKET sockfd = socklist_[index];

            WSANETWORKEVENTS event_struct = {};
            if (WSAEnumNetworkEvents(sockfd, hEvent, &event_struct) == SOCKET_ERROR)
            {
                _tprintf(_T("WSAEnumNetworkEvents() failed, %s"), LAST_ERROR_MSG);
                on_close(sockfd, 0);
                continue;
            }

            // handle socket events here
            event_handler(sockfd, &event_struct);
        }
    }
}

int  worker::event_handler(SOCKET sockfd, const WSANETWORKEVENTS* events_struct)
{
    const int* errorlist = events_struct->iErrorCode;
    int events = events_struct->lNetworkEvents;
    if (events & FD_READ)
    {
        on_recv(sockfd, errorlist[FD_READ_BIT]);
    }
    if (events & FD_WRITE)
    {
        on_write(sockfd, errorlist[FD_WRITE_BIT]);
    }
    if (events & FD_CLOSE)
    {
        on_close(sockfd, errorlist[FD_CLOSE_BIT]);
    }
    return 1;
}

bool worker::on_recv(SOCKET sockfd, int error)
{
    char databuf[BUFE_SIZE];
    int bytes = recv(sockfd, databuf, BUFE_SIZE, 0);
    if (bytes == SOCKET_ERROR && bytes == 0)
    {
        return on_close(sockfd, 0);
    }

    // send back
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        return on_close(sockfd, 0);
    }

    return true;
}

bool worker::on_write(SOCKET sockfd, int error)
{
    UNREFERENCED_PARAMETER(sockfd);

    // do nothing here
    return true;
}


bool worker::on_close(SOCKET sockfd, int error)
{
    size_t index = std::find(socklist_, socklist_+count_, sockfd) - socklist_;
    if (index < 0 || index > count_)
    {
        _tprintf(_T("socket %d not found.\n"), sockfd);
        return false;
    }

    WSAEVENT hEvent = eventlist_[index];
    WSAEventSelect(sockfd, NULL, 0); // cancel event association
    WSACloseEvent(hEvent);
    closesocket(sockfd);

    std::remove(socklist_, socklist_+count_, sockfd);
    std::remove(eventlist_, eventlist_+count_, hEvent);
    --count_;

    _tprintf(_T("socket %d closed at %s.\n"), sockfd, Now().data());
    return true;
}

void worker::reset()
{
    for (size_t i = 0; i < count_; ++i)
    {
        WSAEventSelect(socklist_[i], NULL, 0); // cancel event association
        WSACloseEvent(eventlist_[i]);
        closesocket(socklist_[i]);
    }

    for (size_t i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
    {
        eventlist_[i] = WSA_INVALID_EVENT;
        socklist_[i] = INVALID_SOCKET;
    }

    count_ = 0;
}

// communicate with other thread(s)
bool worker::handle_messages()
{
    SOCKET sockfd = INVALID_SOCKET;
    MSG msg = {};
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if (GetMessage(&msg, NULL, 0, 0))
        {
            switch(msg.message)
            {
            case WM_ADD_NEW_SOCKET:
                sockfd = msg.wParam;
                break;
            }
        }
    }

    if (sockfd == INVALID_SOCKET)
    {
        return false;
    }

    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        _tprintf(_T("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    // associate event object to this socket
    if (WSAEventSelect(sockfd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        WSACloseEvent(hEvent);
        _tprintf(_T("WSAEventSelect() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    _tprintf(_T("socket %d connected at %s.\n"), sockfd, Now().data());
    eventlist_[count_] = hEvent;
    socklist_[count_] = sockfd;
    count_++;

    return true;
}
