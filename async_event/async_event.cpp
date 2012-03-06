
#include "async_event.h"
#include <algorithm>


async_event_server::async_event_server()
    : total_event_(0), listen_socket_(INVALID_SOCKET)
{
    memset(event_list_, 0, sizeof(event_list_));
    memset(buffer_list_, 0, sizeof(buffer_list_));
    memset(socket_list_, 1, sizeof(socket_list_));
}

bool async_event_server::start(const TCHAR* host, short port)
{
    if (!create_listen_socket(host, port))
    {
        return false;
    }

    for (;;)
    {
        DWORD index = WSAWaitForMultipleEvents(total_event_, event_list_, FALSE, INFINITE, FALSE);
        if (index == WSA_WAIT_FAILED)
        {
            LOG_PRINT(_T("WSAWaitForMultipleEvents() failed"));
            break;
        }
        if (index >= total_event_)
        {
            continue;
        }

        char* buffer = buffer_list_[index];
        if (buffer == nullptr)
        {
            continue;
        }

        WSAEVENT event = event_list_[index];
        SOCKET socket = socket_list_[index];
        WSAResetEvent(event);

        WSANETWORKEVENTS net_events = {};
        if (WSAEnumNetworkEvents(socket, event, &net_events) == SOCKET_ERROR)
        {
            LOG_PRINT(_T("WSAEnumNetworkEvents() failed"));
            continue;
        }

        int events_flag = net_events.lNetworkEvents;
        if (events_flag & FD_READ)
        {
            on_recv(socket, buffer, net_events.iErrorCode[FD_READ_BIT]);
        }
        if (events_flag & FD_WRITE)
        {
            //
        }
        if (events_flag & FD_ACCEPT)
        {
            on_accepted(socket, net_events.iErrorCode[FD_ACCEPT_BIT]);
        }
        if (events_flag & FD_CLOSE)
        {
            on_close(socket, net_events.iErrorCode[FD_CLOSE_BIT]);
        }
    }

    return 0;
}


void    async_event_server::on_accepted(SOCKET socket, int error_code)
{
    if (error_code)
    {
        LOG_PRINT(_T("on_accepted() error code"));
        on_close(socket, 0);
        return ;
    }

    sockaddr_in peeraddr = {};
    int addrlen = sizeof(peeraddr);
    SOCKET socket_accept = accept(listen_socket_, (sockaddr*)&peeraddr, &addrlen);
    if (socket_accept == INVALID_SOCKET)
    {
        LOG_PRINT(_T("accept() failed"));
        return ;
    }

    if (!dispatch_event(socket_accept, FD_READ | FD_WRITE | FD_CLOSE))
    {
        closesocket(socket_accept);
        return ;
    }
}

void    async_event_server::on_recv(SOCKET socket, char* buffer, int error_code)
{
    if (error_code)
    {
        LOG_PRINT(_T("recv error on %d, %s"), socket, GetErrorMessage(error_code).data());
    }

    int bytes_read = recv(socket, buffer, BUFE_SIZE, 0);
    if (bytes_read == SOCKET_ERROR || bytes_read == 0)
    {
        on_close(socket, 0);
        return ;
    }

    do_send(socket, buffer, bytes_read, 0);
}

void    async_event_server::do_send(SOCKET socket, char* buffer, int msglen, int error_code)
{
    if (error_code)
    {
        LOG_PRINT(_T("send error on %d, %s"), socket, GetErrorMessage(error_code).data());
    }

    int bytes_sent = send(socket, buffer, msglen, 0);
    if (bytes_sent == SOCKET_ERROR)
    {
        on_close(socket, 0);
        return ;
    }
}

void    async_event_server::on_close(SOCKET socket, int error_code)
{
    if (error_code)
    {
        LOG_PRINT(_T("%d closed, %s"), socket, GetErrorMessage(error_code).data());
    }

    closesocket(socket);
    destroy_event(socket);
}



bool  async_event_server::create_listen_socket(const TCHAR* host, short port)
{
    _tstring straddr = host;
    straddr += _T(":");
    straddr += ToString(port);
    sockaddr_in addr = {};
    if (!StringToAddress(straddr, &addr))
    {
        LOG_PRINT(_T("StringToAddress() failed, %s"), straddr.data());
        return false;
    }

    listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket_ == INVALID_SOCKET)
    {
        LOG_PRINT(_T("socket() failed"));
        return false;
    }

    int error = bind(listen_socket_, (sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("bind() failed, %s"), straddr.data());
        closesocket(listen_socket_);
        return false;
    }

    int listen_max = SOMAXCONN > WSA_MAXIMUM_WAIT_EVENTS ? WSA_MAXIMUM_WAIT_EVENTS : SOMAXCONN;
    error = listen(listen_socket_, listen_max);
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("listen() failed"));
        closesocket(listen_socket_);
        return false;
    }

    if (!dispatch_event(listen_socket_, FD_ACCEPT))
    {
        closesocket(listen_socket_);
        return false;
    }

    return true;
}


bool async_event_server::dispatch_event(SOCKET socket, int events_flag)
{
    if (total_event_ == WSA_MAXIMUM_WAIT_EVENTS)
    {
        return false;
    }

    WSAEVENT event = WSACreateEvent();
    if (event == WSA_INVALID_EVENT)
    {
        return false;
    }
    
    char* buffer = nullptr;
    try
    {
        buffer = new char[BUFE_SIZE];
    }
    catch (std::bad_alloc&)
    {
        WSACloseEvent(event);
        return false;
    }

    if (WSAEventSelect(socket, event, events_flag) == SOCKET_ERROR)
    {
        LOG_PRINT(_T("WSAEventSelect() failed, socket: %d"), socket);
        WSACloseEvent(event);
        delete [] buffer;
        return false;
    }
    
    event_list_[total_event_] = event;
    socket_list_[total_event_] = socket;
    buffer_list_[total_event_] = buffer;
    ++total_event_;
    return true;
}

bool async_event_server::destroy_event(SOCKET socket)
{
    SOCKET* position = std::find(socket_list_, socket_list_+total_event_, socket);
    if (position == socket_list_ + total_event_)
    {
        return false;
    }

    int index = position - socket_list_;
    WSACloseEvent(event_list_[index]);
    delete [] buffer_list_[index];
    --total_event_;
    for (int i = index; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
    {
        event_list_[i] = event_list_[i+1];
        socket_list_[i] = socket_list_[i+1];
        buffer_list_[i] = buffer_list_[i+1];
    }

    return true;
}