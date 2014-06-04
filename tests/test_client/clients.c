#include "clients.h"
#include <winsock2.h>
#include <mswsock.h >
#include <assert.h>
#include <stdio.h>
#include "common/avl.h"
#include "common/utility.h"



typedef struct loop_data
{
    LPFN_CONNECTEX      fnConnectEx;
    HANDLE              completion_port;
    avl_tree_t*         total_connection;
    avl_tree_t*         connection_data;
}loop_data_t;


static loop_data_t  default_loop;    /* global internal data */


static SOCKET create_socket()
{
    int error;
    struct sockaddr_in local_addr;
    SOCKET sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    /* ConnectEx() need bind() first */
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    error = bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s."), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }
    return sockfd;
}

/* allocate handle data for socket descriptor */
static PER_HANDLE_DATA* alloc_handle_data(SOCKET sockfd)
{
    PER_HANDLE_DATA* handle_data;
    avl_tree_t* data_map = default_loop.connection_data;

    if (avl_find(data_map, (avl_key_t)sockfd))
    {
        return NULL;
    }
    handle_data = (PER_HANDLE_DATA*)malloc(sizeof(PER_HANDLE_DATA));
    memset(handle_data, 0, sizeof(PER_HANDLE_DATA));
    handle_data->socket = sockfd;
    handle_data->opertype = OperConnect;
    handle_data->wsbuf.buf = handle_data->buffer;
    handle_data->wsbuf.len = sizeof(handle_data->buffer);

    avl_insert(data_map, (avl_key_t)sockfd, handle_data);

    return handle_data;
}

/* free handle data */
static void free_handle_data(PER_HANDLE_DATA* handle_data)
{
    SOCKET sockfd;
    assert(handle_data);
    sockfd = handle_data->socket;
    avl_delete(default_loop.connection_data, (avl_key_t)sockfd);
    free(handle_data);
}

/* connection closed */
static void on_close(PER_HANDLE_DATA* handle_data)
{
    SOCKET sockfd;
    assert(handle_data);
    sockfd = handle_data->socket;
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
    closesocket(sockfd);
    avl_delete(default_loop.total_connection, (avl_key_t)sockfd);
    free_handle_data(handle_data);
}


/* Send a message after connected */
static void  on_connected(PER_HANDLE_DATA* handle_data)
{
    int error;
    DWORD bytes;
    const char* msg = "GET /index.html HTTP/1.0\r\n\r\n";
    size_t len = strlen(msg) + 1;

    assert(handle_data);
    fprintf(stdout, ("socket %d connected at %s.\n"), handle_data->socket, Now());

    memcpy_s(handle_data->buffer, kDefaultBufferSize, msg, len);
    memset(&handle_data->overlap, 0, sizeof(handle_data->overlap));
    handle_data->wsbuf.len = len;
    handle_data->opertype = OperSend;

    error = WSASend(handle_data->socket, &handle_data->wsbuf, 1, &bytes,
        0, &handle_data->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSASend() failed, socket: %d, %d: %s", handle_data->socket,
                error, LAST_ERROR_MSG);
            on_close(handle_data);
        }
        on_close(handle_data);
    }
}

/* Send message back after recieved */
static void  on_recv(PER_HANDLE_DATA* handle_data)
{
    int error;
    DWORD bytes;

    assert(handle_data);

    /* pause */
    Sleep(200);

    bytes = handle_data->overlap.InternalHigh;
    memset(&handle_data->overlap, 0, sizeof(handle_data->overlap));
    handle_data->wsbuf.len = bytes;
    handle_data->wsbuf.buf[bytes] = '\0';
    handle_data->opertype =  OperSend;

    error = WSASend(handle_data->socket, &handle_data->wsbuf, 1, &bytes,
        0, &handle_data->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSASend() failed, socket: %d, %d: %s", handle_data->socket,
                error, LAST_ERROR_MSG);
            on_close(handle_data);
        }
    }
}

static void  after_sent(PER_HANDLE_DATA* handle_data)
{
    int error;
    DWORD bytes = 0;
    DWORD flag = 0;

    assert(handle_data);
    memset(&handle_data->overlap, 0, sizeof(handle_data->overlap));
    handle_data->wsbuf.len = kDefaultBufferSize;
    handle_data->opertype =  OperRecv;
    error = WSARecv(handle_data->socket, &handle_data->wsbuf, 1, &bytes,
        &flag, &handle_data->overlap, NULL);
    if (error == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        if (error != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSARecv() failed, socket: %d, %d: %s", handle_data->socket,
                error, LAST_ERROR_MSG);
            on_close(handle_data);
        }
    }
}

int  create_connections(const char* host, short port, int num)
{
    int i;
    int error;
    int count;
    SOCKET sockfd;
    PER_HANDLE_DATA* handle_data;
    HANDLE completion_port;
    struct sockaddr_in remote_addr;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(host);
    remote_addr.sin_port = htons(port);

    count = 0;
    completion_port = default_loop.completion_port;
    for (i = 0; i < num; i++)
    {
        sockfd = create_socket();
        if (sockfd == INVALID_SOCKET)
        {
            continue;
        }
        handle_data = alloc_handle_data(sockfd);
        if (handle_data == NULL)
        {
            closesocket(sockfd);
            continue;
        }
        if (!AssociateDevice(completion_port, (HANDLE)sockfd, (ULONG_PTR)handle_data))
        {
            closesocket(sockfd);
            free_handle_data(handle_data);
            continue;
        }
        error = default_loop.fnConnectEx(sockfd, (struct sockaddr*)&remote_addr,
            sizeof(remote_addr), NULL, 0, NULL, &handle_data->overlap);
        if (error == 0 && WSAGetLastError() != WSA_IO_PENDING)
        {
            fprintf(stderr, ("ConnectEx() failed [%d], %s."), sockfd, LAST_ERROR_MSG);
            closesocket(sockfd);
            free_handle_data(handle_data);
            continue;
        }
        count++;
    }
    return count;
}

int  loop_run(int timeout)
{
    int error;
    DWORD bytes = 0;
    PER_HANDLE_DATA* handle_data = NULL;
    WSAOVERLAPPED* overlap = NULL;

    error = GetQueuedCompletionStatus(default_loop.completion_port, &bytes,
        (ULONG_PTR*)&handle_data, &overlap, timeout);
    if (error == 0)
    {
        error = GetLastError();
        if (error == WAIT_TIMEOUT)
        {
            return 1;
        }
        fprintf(stderr, "%d: %s", error, LAST_ERROR_MSG);
        if (overlap != NULL)
        {
            handle_data->opertype = OperClose;
        }
        else
        {
            if (error == ERROR_INVALID_HANDLE)
            {
                return 0;
            }
            return 1;
        }
    }

    switch(handle_data->opertype)
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
        assert(0);
    }
    return 1;
}

int  loop_init()
{
    WSADATA data;
    DWORD bytes = 0;
    int error;
    SOCKET sockfd = INVALID_SOCKET;
    GUID guid_connectex = WSAID_CONNECTEX;

    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("WSAIoctl() failed, %s."), LAST_ERROR_MSG);
        return 0;
    }

    /* obtain ConnectEx function pointer */
    error = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_connectex,
        sizeof(guid_connectex), &default_loop.fnConnectEx, sizeof(default_loop.fnConnectEx),
        &bytes, 0, 0);
    closesocket(sockfd);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAIoctl() failed, %s."), LAST_ERROR_MSG);
        return 0;
    }

    /* connection tree container */
    default_loop.total_connection = avl_create_tree();
    if (default_loop.total_connection == NULL)
    {
        fprintf(stderr, ("avl_create_tree() failed."));
        return 0;
    }

    /* per-handle data */
    default_loop.connection_data = avl_create_tree();
    if (default_loop.connection_data == NULL)
    {
        fprintf(stderr, ("avl_create_tree() failed."));
        avl_destroy_tree(default_loop.connection_data);
        return 0;
    }

    /* create I/O completion port handle */
    default_loop.completion_port = CreateCompletionPort(0);
    if (default_loop.completion_port == NULL)
    {
        fprintf(stderr, ("CreateCompletionPort() failed, %s."), LAST_ERROR_MSG);
        avl_destroy_tree(default_loop.total_connection);
        avl_destroy_tree(default_loop.connection_data);
        return 0;
    }

    return 1;
}

void loop_destroy()
{
    CloseHandle(default_loop.completion_port);
    avl_destroy_tree(default_loop.connection_data);
    avl_destroy_tree(default_loop.total_connection);
}
