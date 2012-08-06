#include "worker.h"
#include "../common/thread.h"
#include <functional>
#include <algorithm>



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
    my_thread_ = create_thread(std::tr1::bind(&worker::main_loop, this));
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
            LOG_PRINT(_T("WSAWaitForMultipleEvents() failed"));
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
                LOG_DEBUG(_T("WSAEnumNetworkEvents() failed"));
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
    if (error)
    {
        LOG_PRINT(_T("Error encountered, ID: %d\n"), error);
        return false;
    }

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
    if (error)
    {
        LOG_PRINT(_T("Error encountered, ID: %d\n"), error);
        return false;
    }

    // do nothing here
    return true;
}


bool worker::on_close(SOCKET sockfd, int error)
{
    if (error)
    {
        LOG_PRINT(_T("Error encountered, ID: %d\n"), error);
        return false;
    }

    size_t index = std::find(socklist_, socklist_+count_, sockfd) - socklist_;
    if (index < 0 || index > count_)
    {
        LOG_PRINT(_T("socket %d not found in list"), sockfd);
        return false;
    }

    WSAEVENT hEvent = eventlist_[index];
    WSAEventSelect(sockfd, NULL, 0); // cancel event association
    WSACloseEvent(hEvent);
    closesocket(sockfd);

    for (size_t i = index; i < count_; ++i)
    {
        eventlist_[i] = eventlist_[i+1];
        socklist_[i] = socklist_[i+1];
    }
    --count_;

    _tprintf(_T("%s, socket %d closed.\n"), Now().data(), sockfd);

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


bool worker::handle_messages()
{
    SOCKET sockfd = INVALID_SOCKET;
    MSG msg = {};
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_ADD_NEW_SOCKET)
            {
                sockfd = msg.wParam;
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
        LOG_DEBUG(_T("WSACreateEvent() failed"));
        return false;
    }

    // associate event object to this socket
    if (WSAEventSelect(sockfd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        WSACloseEvent(hEvent);
        LOG_DEBUG(_T("WSAEventSelect() failed"));
        return false;
    }

    _tprintf(_T("%s, socket %d connected.\n"), Now().data(), sockfd);
    eventlist_[count_] = hEvent;
    socklist_[count_] = sockfd;
    count_++;

    return true;
}
