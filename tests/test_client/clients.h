#pragma once

#include "../../src/common/utility.h"
#include <memory>
#include <map>


// 连接客户端
class clients
{
public:
    clients();
    ~clients();

    // 初始化和事件循环
    bool start(const char* host, short port, int count);

private:
    clients(const clients&);
    clients& operator = (const clients&);

    // 连接成功后的回调处理
    void        on_connected(PER_HANDLE_DATA* handle_data);

    // 收到数据后的回调处理
    void        on_recv(PER_HANDLE_DATA* handle_data);

    // 连接关闭后的回调处理
    void        on_close(PER_HANDLE_DATA* handle_data);

    // 发送完成后的回调处理
    void        after_sent(PER_HANDLE_DATA* handle_data);

    // 创建一个客户端连接
    bool                create_one_client(const sockaddr_in& addr);

    // 申请和释放句柄资源
    PER_HANDLE_DATA*    alloc_handle_data(SOCKET sock_fd);
    void                free_handle_data(PER_HANDLE_DATA* handle_data);

private:
    HANDLE              comletport_handle_;         // I/O完成端口句柄
    LPFN_CONNECTEX      fnConnectEx;                // ConectEx异步连接
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;  // 所有的连接
};