#include "clients.h"
#include <assert.h>


using std::string;



clients::clients()
    : comletport_handle_(INVALID_HANDLE_VALUE),
      fnConnectEx(NULL)
{
}

clients::~clients()
{
    CloseHandle(comletport_handle_);
}

bool clients::start(const char* host, short port, int count)
{
    comletport_handle_ = CreateCompletionPort(0);
    if (comletport_handle_ == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, ("CreateCompletionPort() failed, %s."), LAST_ERROR_MSG);
        return false;
    }

    sockaddr_in remote_addr = {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host);
    remote_addr.sin_port = htons(port);

    for (int i = 0; i < count; ++i)
    {
        create_one_client(remote_addr);
    }

    DWORD bytes_transferred = 0;
    PER_HANDLE_DATA* handle_data = NULL;
    WSAOVERLAPPED* overlap = NULL;
    for (;;)
    {
        int error = GetQueuedCompletionStatus(comletport_handle_, &bytes_transferred, 
            (ULONG_PTR*)&handle_data, &overlap, 500);
        if (error == 0)
        {
            if (overlap == NULL) 
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

// 连接成功后发送一条消息
void  clients::on_connected(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    fprintf(stdout, ("socket %d connected at %s.\n"), handle_data->socket_, Now().data());
    char szmsg[MAX_PATH] = ("The quick fox jumps over a lazy dog");
    size_t len = strlen(szmsg) + 1;
    handle_data->wsbuf_.len = len;
    memcpy_s(handle_data->buffer_, sizeof(handle_data->buffer_), szmsg, len);
    handle_data->opertype_ =  OperSend;
    DWORD bytes_send = 0;
    int error = WSASend(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_send,
        0, &handle_data->overlap_, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

// 收到Server返回后再次发送
void  clients::on_recv(PER_HANDLE_DATA* handle_data)
{
    // 暂停1秒
    ::Sleep(1000);

    assert(handle_data);
    DWORD recv_bytes = handle_data->overlap_.InternalHigh;
    handle_data->wsbuf_.len = recv_bytes;
    char* pbuf = (char*)handle_data->wsbuf_.buf;
    pbuf[recv_bytes/sizeof(char)] = ('\0');
    fprintf(stdout, ("message of socket %d: %s.\n"), handle_data->socket_, pbuf);

    handle_data->opertype_ =  OperSend;
    DWORD bytes_send = 0;
    int error = WSASend(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_send,
        0, &handle_data->overlap_, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

// 发送成功后发起读取请求
void  clients::after_sent(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);    
    handle_data->wsbuf_.len = sizeof(handle_data->buffer_);
    handle_data->opertype_ =  OperRecv;
    DWORD bytes_recv = 0;
    DWORD flag = 0;
    int error = WSARecv(handle_data->socket_, &handle_data->wsbuf_, 1, &bytes_recv,
        &flag, &handle_data->overlap_, NULL);
    if (error == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        on_close(handle_data);
    }
}

// 关闭套接字，释放资源
void  clients::on_close(PER_HANDLE_DATA* handle_data)
{
    assert(handle_data);
    fprintf(stdout, ("socket %d closed at %s.\n"), handle_data->socket_, Now().data());
    closesocket(handle_data->socket_);
    free_handle_data(handle_data);
}

// 创建一个客户端连接
bool clients::create_one_client(const sockaddr_in& remote_addr)
{
    SOCKET sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed"));
        return false;
    }

    if (fnConnectEx == NULL)
    {
        GUID guid_connectex = WSAID_CONNECTEX;
        DWORD bytes = 0;
        int error = WSAIoctl(sock_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connectex,
            sizeof(guid_connectex), &fnConnectEx, sizeof(fnConnectEx), &bytes, 0, 0);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, ("WSAIoctl() failed, %s."), LAST_ERROR_MSG);
            closesocket(sock_fd);
            return false;
        }
    }

    // ConnectEx()需要本地套接字先bind()
    sockaddr_in localaddr = {};
    localaddr.sin_family = AF_INET;
    int error = bind(sock_fd, (sockaddr*)&localaddr, sizeof(localaddr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s."), LAST_ERROR_MSG);
        closesocket(sock_fd);
        return false;
    }

    PER_HANDLE_DATA* handle_data = alloc_handle_data(sock_fd);
    if (handle_data == NULL)
    {
        closesocket(sock_fd);
        return false;
    }

    if (!AssociateDevice(comletport_handle_, (HANDLE)sock_fd, (ULONG_PTR)handle_data))
    {
        fprintf(stderr, ("AssociateDevice() failed [%d], %s."), sock_fd, LAST_ERROR_MSG);
        closesocket(sock_fd);
        free_handle_data(handle_data);
        return false;
    }
    
    BOOL bOK = fnConnectEx(sock_fd, (sockaddr*)&remote_addr, sizeof(remote_addr), NULL, 0,
        NULL, &handle_data->overlap_);
    if (!bOK && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, ("ConnectEx() failed [%d], %s."), sock_fd, LAST_ERROR_MSG);
        closesocket(sock_fd);
        free_handle_data(handle_data);
        return false;
    }

    return true;
}


// 申请客户端连接所需的资源
PER_HANDLE_DATA* clients::alloc_handle_data(SOCKET sock_fd)
{
    if (info_map_.count(sock_fd))
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
        fprintf(stderr, ("allocate handle data failed.\n"));
        return NULL;
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