/**
 *  @file   iocp.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用I/O完成端口多线程模式实现的简单Echo Server
 * 
 */
#pragma once

#include <map>
#include <list>
#include <queue>
#include <memory>
#include "../common/utility.h"



class iocp_server
{
public:
    iocp_server();
    ~iocp_server();

    // 开始运行
    bool    start(const char* host, int port);

    // 获取I/O完成端口句柄
    HANDLE  completion_port() {return completion_port_;}

    // 命令队列
    void    push_command(PER_HANDLE_DATA* handle_data);    

private:
    iocp_server(const iocp_server&);
    iocp_server& operator = (const iocp_server&);

    // 事件处理
    void    on_accepted(PER_HANDLE_DATA* handle_data);
    void    on_recv(PER_HANDLE_DATA* handle_data);
    void    on_sent(PER_HANDLE_DATA* handle_data);
    void    on_disconnect(PER_HANDLE_DATA* handle_data);
    void    on_closed(PER_HANDLE_DATA* handle_data);
    
    // 事件循环
    bool    event_loop();

    // 创建worker线程
    bool    create_workers(DWORD concurrency);   

    // 创建监听套接字
    bool    create_listen_socket(const char* host, int port);

    // 发起异步accept请求
    bool    post_an_accept(); 

    PER_HANDLE_DATA*    pop_command();
    

    // 资源申请和释放
    PER_HANDLE_DATA*    alloc_socket_handle();
    void                free_socket_handle(PER_HANDLE_DATA* handle_data);

private:    
    SOCKET      listen_socket_;         // 监听套接字    
    HANDLE      completion_port_;       // 完成端口句柄
           
    std::vector<unsigned>   workers_;   // worker线程
    
    Mutex   mutex_;         // 同步锁
    std::queue<PER_HANDLE_DATA*>    command_queue_;     // 与worker线程通信的命令队列   
    
    std::list<PER_HANDLE_DATA*>         free_list_;     // 空闲列表    
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;      // 
};
