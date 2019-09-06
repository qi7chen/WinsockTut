# WinsockTut

尝试用不同的Windows API实现网络I/O多路复用模型
    

### Overview

I/O Model                |  Source File                   | Description
------------------------ |--------------------------------|---------------------------------------------------------
select() poller          |  src/SelectPoller.cpp          | Linux和Windows都支持的API，大多数的跨平台网络库都会采用
WSAAsyncSelect() poller  |  src/AsyncSelectPoller.cpp     | 网络事件挂接到HWND消息
WSAEventSelect() poller  |  src/AsyncEventPoller.cpp      | 通过事件对象接收网络事件通知，64限制
alertable I/O            |  N/A                           | 本代码尚未实现
overlapped I/O           |  src/OverlappedIOService.cpp   | 重叠I/O，也是通过事件对象接收网络事件通知，也有64限制，鸡肋
I/O Completion Port      |  src/CompletionPortService.cpp | Windows下性能最好的网络I/O多路复用模型


### Introduction

所有的网络多路复用根据使用方式分别采用[Reactor](https://en.wikipedia.org/wiki/Reactor_pattern)和[Proactor](https://en.wikipedia.org/wiki/Proactor_pattern)的方式，做了简单抽象，根据简单的参数应用层示例即可选择不同的模型。


#### Reactor

select, WSAAsyncSelect和WSAEventSelect均采用reactor模型实现。

reactor的API参考了[zeromq](https://github.com/zeromq/zeromq4-1/blob/master/src/select.hpp)
如下所示:

```Cpp
class PollerBase 
{
    int AddFd(SOCKET fd, IPollEvent* event);    // 添加一个套接字fd到事件循环
    void RemoveFd(SOCKET fd);                   // 从事件循环中删除一个套接字fd
    void SetPollIn(SOCKET fd);                  // 设置关注套接字的读事件
    void ResetPollIn(SOCKET fd);                // 设置取消关注套接字的读事件
    void SetPollOut(SOCKET fd);                 // 设置关注套接字的写事件
    void ResetPollOut(SOCKET fd);               // 设置取消关注套接字的写事件
    int Poll(int timeout);                      // 事件循环
}
```

#### Proactor

I/O Completion Port的API参考了boost.asio

如下所示：

```Cpp
class IOServiceBase
{
    int AsyncConnect(OverlapContext* ctx, const addrinfo*); // 连接一个addrinfo指定的地址， ctx中指定回调
    int AsyncAccept(OverlapContext* ctx);                   // accpet一个新套接字fd， ctx中指定回调
    int AsyncRead(OverlapContext* ctx);                     // 异步读网络数据， ctx中指定回调
    int AsyncWrite(OverlapContext* ctx);                    // 异步写网络数据， ctx中指定回调
    int Run(int timeout);                                   // 事件循环
}
```

### 如何构建

* 安装[Visual Studio](https://visualstudio.microsoft.com)
* 安装[CMake](https://cmake.org/download/)
* 使用CMake生成Visual Studio工程文件
* 使用Visual Studio打开工程


### Example 

使用Reactor模型的echo示例

[echo示例](https://github.com/ichenq/WinsockTut/tree/master/examples/echo)

使用Proactor模型的ping/pong示例

[pingpong示例](https://github.com/ichenq/WinsockTut/tree/master/examples/pingpong)


### 测试
[tests](https://github.com/ichenq/WinsockTut/tree/master/tests)目录下是用golang实现的echo server和echo client



### 备注

本代码仅作学习参考或者教学目的
