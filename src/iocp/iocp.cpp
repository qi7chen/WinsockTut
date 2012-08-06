
#include "iocp.h"
#include "../common/logging.h"
#include <MSWSock.h>
#include <functional>
#include "worker.h"


#pragma warning(disable: 4127)


namespace {

struct accept_info
{
    SOCKET socket_accept_;
    char addrbuf_[BUFE_SIZE - sizeof(SOCKET)];
};

static const DWORD ADDR_LEN = sizeof(sockaddr_in) + 16;

static LPFN_ACCEPTEX                fnAcceptEx;
static LPFN_GETACCEPTEXSOCKADDRS    fnGetAcceptExSockaddrs;
static LPFN_DISCONNECTEX            fnDisconnectEx;



// obtain function address at run time
bool init_extend_function_poiner(SOCKET socket)
{
    DWORD bytes;
    GUID guid_acceptex = WSAID_ACCEPTEX;
    GUID guid_getacceptexaddr = WSAID_GETACCEPTEXSOCKADDRS;
    GUID guid_disconnectex = WSAID_DISCONNECTEX;

    int error = ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_acceptex,
                           sizeof(guid_acceptex), &fnAcceptEx, sizeof(fnAcceptEx), &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        LOG_ERROR(_T("get AcceptEx pointer failed"));
        return false;
    }

    error = ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_getacceptexaddr,
                       sizeof(guid_getacceptexaddr), &fnGetAcceptExSockaddrs, sizeof(fnGetAcceptExSockaddrs), &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        LOG_ERROR(_T("get GetAcceptExSockaddrs pointer failed"));
        return false;
    }

    error = ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_disconnectex,
                       sizeof(guid_disconnectex), &fnDisconnectEx, sizeof(fnDisconnectEx), &bytes, NULL, NULL);
    if (error == SOCKET_ERROR)
    {
        LOG_ERROR(_T("get DisconnectEx pointer failed"));
        return false;
    }

    return true;
}

} // anonymous namespace


// ctor
iocp_server::iocp_server()
    : completion_port_(INVALID_HANDLE_VALUE),
      listen_socket_(INVALID_SOCKET)
{
}

iocp_server::~iocp_server()
{
    for (std::map<SOCKET, PER_HANDLE_DATA*>::iterator iter = info_map_.begin(); 
        iter != info_map_.end(); ++iter)
    {
        closesocket(iter->first);
        delete iter->second;
    }
    for (std::list<PER_HANDLE_DATA*>::iterator iter = free_list_.begin();
        iter != free_list_.end(); ++iter)
    {
        closesocket((*iter)->socket_);
        delete *iter;
    }
    ::CloseHandle(completion_port_);
}


// start server
bool iocp_server::start(const TCHAR* host, short port)
{
    if (!create_completion_port(0))
    {
        LOG_ERROR(_T("CreateIoCompletionPort() failed"));
        return false;
    }

    if (!create_workers())
    {
        return false;
    }

    _tprintf(_T("created %u worker thread(s).\n"), workers_.size());

    _tstring straddr = host;
    straddr += _T(":");
    straddr += ToString(port);
    sockaddr_in addr = {};
    if (!StringToAddress(straddr, &addr))
    {
        LOG_ERROR(_T("StringToAddress() failed, %s"), straddr.data());
        return false;
    }

    if (!create_listen_socket(addr))
    {
        return false;
    }

    if (!init_extend_function_poiner(listen_socket_))
    {
        return false;
    }

    if (!post_an_accept())
    {
        return false;
    }

    wait_loop();

    return true;
}


bool iocp_server::create_completion_port(unsigned concurrency)
{
    completion_port_ = CreateCompletionPort(concurrency);
    return completion_port_ != INVALID_HANDLE_VALUE;
}

// create worker threads
bool iocp_server::create_workers(DWORD concurrency /* = 0 */)
{
    if (concurrency == 0)
    {
        SYSTEM_INFO info = {};
        ::GetSystemInfo(&info);
        concurrency = info.dwNumberOfProcessors;
    }
    for (unsigned i = 0; i < concurrency; ++i)
    {
        unsigned thread_id = create_thread(std::tr1::bind(run_worker_loop, this));
        workers_.push_back(thread_id);
    }

    return !workers_.empty();
}


// create a listen socket and associate to completion port
bool  iocp_server::create_listen_socket(const sockaddr_in& addr)
{
    PER_HANDLE_DATA* handle_data = alloc_socket_handle();
    if (handle_data == NULL)
    {
        return false;
    }

    listen_socket_ = handle_data->socket_;
    handle_data->opertype_ = OperAccept;

    int error = ::bind(listen_socket_, (sockaddr*)&addr, sizeof(addr));
    if (error != 0)
    {
        LOG_ERROR(_T("bind() failed"));
        free_socket_handle(handle_data);
        listen_socket_ = INVALID_SOCKET;
        return false;
    }

    int status = ::listen(listen_socket_, SOMAXCONN);
    if (status == SOCKET_ERROR)
    {
        free_socket_handle(handle_data);
        listen_socket_ = INVALID_SOCKET;
        return false;
    }

    _tprintf(_T("server listen at %s\n"), AddressToString(addr).data());

    return true;
}



// post another asyn accept request
bool iocp_server::post_an_accept()
{
    std::map<SOCKET, PER_HANDLE_DATA*>::iterator iter = info_map_.find(listen_socket_);
    if (iter == info_map_.end())
    {
        return false;
    }

    PER_HANDLE_DATA* listen_handle = iter->second;

    PER_HANDLE_DATA* accept_handle = alloc_socket_handle();
    if (accept_handle == NULL)
    {
        return false;
    }

    SOCKET accept_socket = accept_handle->socket_;
    accept_info* info_ptr = (accept_info*)listen_handle->buffer_;
    info_ptr->socket_accept_ = accept_socket;

    int error = fnAcceptEx(listen_handle->socket_, accept_socket, &info_ptr->addrbuf_, 0,
                           ADDR_LEN, ADDR_LEN, NULL, &listen_handle->overlap_);
    if (error == FALSE && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        free_socket_handle(accept_handle);
        return false;
    }

    return true;
}



// allocate bookkeeping resource
PER_HANDLE_DATA* iocp_server::alloc_socket_handle()
{
    if (free_list_.empty())
    {
        SOCKET fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd == INVALID_SOCKET)
        {
            return NULL;
        }

        PER_HANDLE_DATA* handle_data = NULL;
        try
        {
            handle_data = new PER_HANDLE_DATA();
        }
        catch (std::bad_alloc&)
        {
            return NULL;
        }
        if (!AssociateDevice(completion_port(), (HANDLE)fd, (ULONG_PTR)handle_data))
        {
            ::closesocket(fd);
            delete handle_data;
            return NULL;
        }

        handle_data->socket_ = fd;
        handle_data->opertype_ = OperClose; // default oper type
        handle_data->buffer_[0] = _T('\0');
        handle_data->wsbuf_.buf = handle_data->buffer_;
        handle_data->wsbuf_.len = sizeof(handle_data->buffer_);

        free_list_.push_back(handle_data);
    }

    PER_HANDLE_DATA* handle_data = free_list_.front();
    free_list_.pop_front();
    info_map_[handle_data->socket_] = handle_data;
    return handle_data;
}

// add handle data to free list
void iocp_server::free_socket_handle(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    if (std::find(free_list_.begin(), free_list_.end(), handle_data) != free_list_.end())
    {
        LOG_ERROR(_T("socket %d handle already in free list"), handle_data->socket_);
        return ;
    }

    handle_data->opertype_ = OperClose;
    info_map_.erase(handle_data->socket_);
    free_list_.push_back(handle_data);
}




//////////////////////////////////////////////////////////////////////////

// new connection accepted
void    iocp_server::on_accepted(PER_HANDLE_DATA* listen_handle)
{
    assert(listen_handle);
    accept_info* info_ptr = (accept_info*)listen_handle->buffer_;
    SOCKET socket_accept = info_ptr->socket_accept_;
    sockaddr_in* remote_addr_ptr = NULL;
    sockaddr_in* local_addr_ptr = NULL;
    int remote_len = sizeof(sockaddr_in);
    int local_len = sizeof(sockaddr_in);
    fnGetAcceptExSockaddrs(info_ptr->addrbuf_, 0, ADDR_LEN, ADDR_LEN, (sockaddr**)&local_addr_ptr,
                           &local_len, (sockaddr**)&remote_addr_ptr, &remote_len);

    std::map<SOCKET, PER_HANDLE_DATA*>::iterator iter = info_map_.find(socket_accept);
    if (iter == info_map_.end())
    {
        LOG_ERROR(_T("accepted socket %d not found in map"), socket_accept);
        return ;
    }

    PER_HANDLE_DATA* accept_handle = iter->second;

    // deliver another overlap request
    DWORD read_bytes = 0;
    DWORD flag = 0;
    accept_handle->opertype_ = OperRecv;
    int error = ::WSARecv(accept_handle->socket_, &accept_handle->wsbuf_, 1, &read_bytes,
                          &flag, &accept_handle->overlap_, NULL);
    if ((error == 0 && read_bytes == 0) ||
            (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING))
    {
        on_closed(accept_handle);
        return;
    }

    _tprintf(_T("%s accepted\n"), AddressToString(*remote_addr_ptr).data());

    post_an_accept();
}

// socket recieved data
void iocp_server::on_recv(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    DWORD bytes_transferred = handle_data->overlap_.InternalHigh;
    handle_data->wsbuf_.len = bytes_transferred;
    handle_data->opertype_ = OperSend;
    int error = ::WSASend(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_transferred,
                          0, &handle_data->overlap_, NULL);
    if (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
    {
        on_closed(handle_data);
    }
}

// socket sent data
void  iocp_server::on_sent(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    DWORD bytes_read = 0;
    DWORD flag = 0;
    handle_data->opertype_ = OperRecv;
    handle_data->wsbuf_.len = sizeof(handle_data->buffer_);
    int error = ::WSARecv(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_read,
                          &flag, &handle_data->overlap_, NULL);
    if ((error == 0 && bytes_read == 0) ||
            (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING))
    {
        on_closed(handle_data);
    }
}

void iocp_server::on_disconnect(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    handle_data->opertype_ = OperClose;
    int status = fnDisconnectEx(handle_data->socket_, &handle_data->overlap_, TF_REUSE_SOCKET, 0);
    if (status == FALSE && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_closed(handle_data);
    }
}

// socket was closed
void  iocp_server::on_closed(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    _tprintf(_T("%d closed\n"), handle_data->socket_);
    free_socket_handle(handle_data);
}




void    iocp_server::wait_loop()
{
    for (;;)
    {
        PER_HANDLE_DATA* handle_data = NULL;
        {
            auto_lock lock(mutex_);
            if (!command_queue_.empty())
            {
                handle_data = pop_command();
            }
        }
        if (handle_data == NULL)
        {
            ::Sleep(1);
            continue;
        }

        assert(handle_data);
        switch(handle_data->opertype_)
        {
            case OperAccept:
                on_accepted(handle_data);
                break;

            case OperRecv:
                on_recv(handle_data);
                break;

            case OperSend:
                on_sent(handle_data);
                break;

            case OperDisconnect:
                on_disconnect(handle_data);
                break;

            case OperClose:
                on_closed(handle_data);
                break;

            default:
                assert(false);
        }
    }
}

void    iocp_server::push_command(PER_HANDLE_DATA* handle_data)
{
    if (handle_data)
    {
        auto_lock lock(mutex_);
        command_queue_.push(handle_data);
    }
}

PER_HANDLE_DATA*  iocp_server::pop_command()
{
    PER_HANDLE_DATA* handle_data = NULL;
    auto_lock lock(mutex_);
    if (!command_queue_.empty())
    {
        handle_data = command_queue_.front();
        command_queue_.pop();
    }
    return handle_data;
}
