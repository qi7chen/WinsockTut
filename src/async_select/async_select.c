/**
 *  @file   async_select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  a simple echo server implemented by WSAAsyncSelect()
 *
 */

#include <stdio.h>
#include "appdef.h"
#include "common/avl.h"
#include "common/utility.h"


static avl_tree_t*  g_total_connections; /* total client connections */


static int on_accepted(HWND hwnd, SOCKET acceptor)
{
    int error;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET sockfd = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("accpet() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    error = WSAAsyncSelect(sockfd, hwnd, WM_SOCKET, FD_WRITE|FD_READ|FD_CLOSE);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return 0;
    }

    avl_insert(g_total_connections, sockfd, NULL);
    fprintf(stdout, ("socket %d accepted at %s.\n"), sockfd, Now());
    return 1;
}

static void on_closed(SOCKET sockfd)
{
    closesocket(sockfd);
    avl_delete(g_total_connections, sockfd);
    fprintf(stdout, ("socket %d closed at %s.\n"), sockfd, Now());
}

static int on_recv(SOCKET sockfd)
{
    char databuf[kDefaultBufferSize];
    int bytes = recv(sockfd, databuf, kDefaultBufferSize, 0);
    if (bytes == SOCKET_ERROR || bytes == 0)
    {
        on_closed(sockfd);
        return 0;
    }

    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        on_closed(sockfd);
        return 0;
    }
    return 1;
}

int InitializeServer(HWND hwnd, const char* host, int port)
{
    int error;
    WSADATA data;
    SOCKET acceptor;
    struct sockaddr_in addr;

    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);

    g_total_connections = avl_create_tree();
    CHECK(g_total_connections != NULL);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);
    acceptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (acceptor == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return 0;
    }

    error = bind(acceptor, (struct sockaddr*)&addr, sizeof(addr));
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return 0;
    }

    error = listen(acceptor, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return 0;
    }

    /* http://msdn.microsoft.com/en-us/library/windows/desktop/ms741540(v=vs.85).aspx
     *
     * The WSAAsyncSelect function automatically sets socket s to nonblocking mode,
     * regardless of the value of lEvent.
     */
    if (WSAAsyncSelect(acceptor, hwnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
    {
        fprintf(stderr, ("WSAAsyncSelect() failed, %s"), LAST_ERROR_MSG);
        closesocket(acceptor);
        return 0;
    }

    avl_insert(g_total_connections, acceptor, NULL);
    fprintf(stdout, ("server listen at %s:%d, %s.\n"), host, port, Now());

    return 1;
}

void CloseServer()
{
    int i;
    int count = avl_size(g_total_connections);
    int* array = (int*)malloc(count * sizeof(int));
    avl_serialize(g_total_connections, array, count);
    for (i = 0; i < count; ++i)
    {
        on_closed(array[i]);
    }
    free(array);
    WSACleanup();
    fprintf(stdout, ("server[%d] closed at %s.\n"), count, Now());
}

int HandleNetEvents(HWND hwnd, SOCKET sockfd, int event, int error)
{
    if (error)
    {
        on_closed(sockfd);
        return 0;
    }
    switch(event)
    {
    case FD_ACCEPT:
        on_accepted(hwnd, sockfd);
        break;
    case FD_READ:
        on_recv(sockfd);
        break;
    case FD_WRITE:
        /* do nothing */
        break;
    case FD_CLOSE:
        on_closed(sockfd);
        break;
    default:
        break;
    }
    return 1;
}