
#include "clients.h"
#include <WS2tcpip.h>




clients::clients()
    : comletport_handle_(INVALID_HANDLE_VALUE),
      fnconnectex_(nullptr)
{
}

clients::~clients()
{
    CloseHandle(comletport_handle_);
}

bool clients::start(const TCHAR* host, short port, int count)
{
    comletport_handle_ = CreateCompletionPort(0);
    if (comletport_handle_ == INVALID_HANDLE_VALUE)
    {
        LOG_PRINT(_T("CreateCompletionPort() failed"));
        return false;
    }

    _tstring straddr = host;
    straddr += _T(":");
    straddr += ToString(port);
    sockaddr_in remote_addr = {};
    if (!StringToAddress(straddr, &remote_addr))
    {
        LOG_PRINT(_T("StringToAddress() failed"));
        return false;
    }

    for (int i = 0; i < count; ++i)
    {
        create_one_client(remote_addr);
    }

    DWORD bytes_transferred = 0;
    PER_HANDLE_DATA* handle_data = nullptr;
    WSAOVERLAPPED* overlap = nullptr;
    for (;;)
    {
        int error = GetQueuedCompletionStatus(comletport_handle_, &bytes_transferred, 
            (ULONG_PTR*)&handle_data, &overlap, INFINITE);
        if (error == 0)
        {
            if (overlap == nullptr) 
                break;
            handle_data->opertype_ = OperClose;
        }

        switch(handle_data->opertype_)
        {
        case OperConnect:
            on_connected(handle_data);
            break;

        case OperRecv:
            on_recv(handle_data);
            break;

        case OperSend:
            after_sent(handle_data);
            break;

        case OperClose:
            on_close(handle_data);
            break;

        default:
            assert(false);
        }
    }

    return true;
}

void  clients::on_connected(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    static const _tstring& strmsg = _T("The quick fox jumps over a lazy dog");
    handle_data->wsbuf_.len = strmsg.length();
    memcpy_s(handle_data->buffer_, sizeof(handle_data->buffer_), strmsg.data(), 
        strmsg.length() * sizeof(TCHAR));
    handle_data->opertype_ =  OperSend;
    DWORD bytes_send = 0;
    int error = WSASend(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_send,
        0, &handle_data->overlap_, nullptr);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

void  clients::on_recv(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    DWORD recv_bytes = handle_data->overlap_.InternalHigh;
    handle_data->wsbuf_.len = recv_bytes;
    handle_data->opertype_ =  OperSend;
    DWORD bytes_send = 0;
    int error = WSASend(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_send,
        0, &handle_data->overlap_, nullptr);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

void  clients::after_sent(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);    
    handle_data->wsbuf_.len = sizeof(handle_data->buffer_);
    handle_data->opertype_ =  OperRecv;
    DWORD bytes_recv = 0;
    DWORD flag = 0;
    int error = WSARecv(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_recv,
        &flag, &handle_data->overlap_, nullptr);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

void  clients::on_close(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    closesocket(handle_data->socket_);
    free_handle_data(handle_data);
}



bool clients::create_one_client(const sockaddr_in& remote_addr)
{
    SOCKET sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == INVALID_SOCKET)
    {
        LOG_PRINT(_T("socket() failed"));
        return false;
    }

    if (fnconnectex_ == nullptr)
    {
        GUID guid_connectex = WSAID_CONNECTEX;
        DWORD bytes = 0;
        int error = WSAIoctl(sock_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connectex,
            sizeof(guid_connectex), &fnconnectex_, sizeof(fnconnectex_), &bytes, 0, 0);
        if (error == SOCKET_ERROR)
        {
            LOG_PRINT(_T("WSAIoctl() failed"));
            closesocket(sock_fd);
            return false;
        }
    }

    sockaddr_in localaddr = {};
    localaddr.sin_family = AF_INET;
    int error = bind(sock_fd, (sockaddr*)&localaddr, sizeof(localaddr));
    if (error == SOCKET_ERROR)
    {
        LOG_PRINT(_T("bind() failed"));
        closesocket(sock_fd);
        return false;
    }

    PER_HANDLE_DATA* handle_data = alloc_handle_data(sock_fd);
    if (handle_data == nullptr)
    {
        closesocket(sock_fd);
        return false;
    }

    if (!AssociateDevice(comletport_handle_, (HANDLE)sock_fd, (ULONG_PTR)handle_data))
    {
        LOG_PRINT(_T("AssociateDevice() failed, fd: %d"), sock_fd);
        closesocket(sock_fd);
        free_handle_data(handle_data);
        return false;
    }
    
    BOOL bOK = fnconnectex_(sock_fd, (sockaddr*)&remote_addr, sizeof(remote_addr), nullptr, 0,
        nullptr, &handle_data->overlap_);
    if (!bOK && WSAGetLastError() != WSA_IO_PENDING)
    {
        LOG_PRINT(_T("ConnectEx() failed, fd: %d"), sock_fd);
        closesocket(sock_fd);
        free_handle_data(handle_data);
        return false;
    }

    return true;
}

PER_HANDLE_DATA* clients::alloc_handle_data(SOCKET sock_fd)
{
    if (info_map_.count(sock_fd))
    {
        return nullptr;
    }

    PER_HANDLE_DATA* handle_data = nullptr;
    try
    {
        handle_data = new PER_HANDLE_DATA();
    }
    catch (std::bad_alloc&)
    {
        LOG_PRINT(_T("allocate handle data failed"));       
        return nullptr;
    }

    memset(&handle_data->overlap_, 0, sizeof(handle_data->overlap_));
    handle_data->socket_ = sock_fd;
    handle_data->opertype_ = OperConnect;
    handle_data->wsbuf_.buf = handle_data->buffer_;
    handle_data->wsbuf_.len = sizeof(handle_data->buffer_);
    info_map_[sock_fd] = handle_data;
    return handle_data;
}

void    clients::free_handle_data(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data); 
    info_map_.erase(handle_data->socket_);
    free(handle_data);
}