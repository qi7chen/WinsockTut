/**
 *  @file   async_select.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用WSAEventSelect模型实现的简单Echo Server
 *			事件对象数量受64限制
 */

#include "../common/utility.h"
#include <stdio.h>
#include <map>


// 所有的网络事件句柄
static std::map<SOCKET, WSAEVENT>  g_event_list;

// 所有的客户端套接字句柄
static std::map<WSAEVENT, SOCKET>  g_socket_list;


// 创建监听套接字(非阻塞)
SOCKET create_listen_socket(const char* host, int port)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons((short)port);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        fprintf(stderr, ("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, ("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR)
    {
        fprintf(stderr, ("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    // 将套接字设置为非阻塞模式
    ULONG nonblock = 1;
    if (ioctlsocket(sockfd, FIONBIO, &nonblock) == SOCKET_ERROR)
    {
        fprintf(stderr, ("ioctlsocket() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }
    fprintf(stdout, ("server start listen [%s:%d] at %s.\n"), host, port, Now().data());

    return sockfd;
}

bool on_close(SOCKET sockfd, int error)
{
    WSAEVENT hEvent = g_event_list[sockfd];
    WSAEventSelect(sockfd, NULL, 0); // 取消事件关联
    WSACloseEvent(hEvent);
    closesocket(sockfd);

    g_event_list.erase(sockfd);
    g_socket_list.erase(hEvent);

    fprintf(stderr, ("socket %d closed at %s.\n"), sockfd, Now().data());
    return true;
}


bool on_recv(SOCKET sockfd, int error)
{
    char databuf[BUFSIZ];
    int bytes = recv(sockfd, databuf, BUFSIZ, 0);
    if (bytes == SOCKET_ERROR && bytes == 0)
    {
        return on_close(sockfd, 0);
    }

    // 发送回执
    bytes = send(sockfd, databuf, bytes, 0);
    if (bytes == 0)
    {
        return on_close(sockfd, 0);
    }

    return true;
}

bool on_write(SOCKET sockfd, int error)
{    
    return true;
}

// 新连接到来，为其关联事件句柄
bool on_accept(SOCKET sockfd)
{
    WSAEVENT hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, ("WSACreateEvent() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    // 关联事件句柄到套接字
    if (WSAEventSelect(sockfd, hEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    {
        WSACloseEvent(hEvent);
        fprintf(stderr, ("WSAEventSelect() failed, %s"), LAST_ERROR_MSG);
        return false;
    }

    if (g_event_list.size() == WSA_MAXIMUM_WAIT_EVENTS)
    {
        WSAEventSelect(sockfd, hEvent, 0);
        WSACloseEvent(hEvent);
        fprintf(stderr, "Got 64 limit.\n");
        return false;
    }
    
    g_event_list[sockfd] = hEvent;
    g_socket_list[hEvent] = sockfd;

    fprintf(stdout, ("socket %d connected at %s.\n"), sockfd, Now().data());
    return true;
}


int  handle_event(SOCKET sockfd, const WSANETWORKEVENTS* events_struct)
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

// 事件循环
bool event_loop()
{
    if (g_event_list.empty())
    {
        ::Sleep(100);
        return true;
    }

    int count = 0;
    WSAEVENT eventlist[WSA_MAXIMUM_WAIT_EVENTS] = {}; 
    for (std::map<SOCKET, WSAEVENT>::const_iterator iter = g_event_list.begin();
        iter != g_event_list.end(); ++iter)
    {
        eventlist[count++] = iter->second;
    }

    // 等待网络事件
    size_t nready = WSAWaitForMultipleEvents(count, eventlist, FALSE, 100, FALSE);            
    if (nready == WSA_WAIT_FAILED)
    {
        fprintf(stderr, ("WSAWaitForMultipleEvents() failed, %s"), LAST_ERROR_MSG);
        return false;
    }
    else if (nready == WSA_WAIT_TIMEOUT)
    {
    }
    else if (nready == WSA_WAIT_IO_COMPLETION)
    {
    }
    else
    {
        size_t index = WSA_WAIT_EVENT_0 + nready;
        if (index >= WSA_MAXIMUM_WAIT_EVENTS)
        {
            fprintf(stderr, "invalid event index: %d.\n", index);
            return true;
        }

        WSAEVENT hEvent = eventlist[index];            
        std::map<WSAEVENT, SOCKET>::const_iterator iter = g_socket_list.find(hEvent);
        if (iter == g_socket_list.end())
        {
            fprintf(stderr, "invalid event object %p.\n", &hEvent);
            return true;
        }
        SOCKET sockfd = iter->second;

        // 枚举网络事件
        WSANETWORKEVENTS event_struct = {};
        if (WSAEnumNetworkEvents(sockfd, hEvent, &event_struct) == SOCKET_ERROR)
        {
            fprintf(stderr, ("WSAEnumNetworkEvents() failed, %s"), LAST_ERROR_MSG);
            on_close(sockfd, 0);
            return true;
        }

        // 处理网络事件
        handle_event(sockfd, &event_struct);
    }
    return true;
}


int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: AsyncEvent [host] [port]\n"));
        return 1;
    }

    WinsockInit init;
    SOCKET sockfd = create_listen_socket(argv[1], atoi(argv[2]));
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    for (;;)
    {
        SOCKET socknew = accept(sockfd, NULL, NULL);
        if (socknew != INVALID_SOCKET)
        {
            if (!on_accept(socknew))
            {
                closesocket(socknew);
            }
        }
        else
        {
            if (GetLastError() == WSAEWOULDBLOCK)
            {
                if (event_loop())
                {
                    continue;
                }
            }
            fprintf(stderr, ("accept() failed, %s"), LAST_ERROR_MSG);
            break;
        }
    }

    return 0;
}

