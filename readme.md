#Winsock Samples
Winsock网络编程示例，使用7种不同的I/O模型来实现一个简单的echo server。



##目录
### build  
[Premake4](http://industriousone.com/premake/download)工程脚本

### src
1. socket
使用基础socket API的示例。

2. select
使用select模型的示例。

3. async_select
使用WSAAsyncSelect()的示例。

4. async_event
使用WSAEventSelect()的示例，实现了一个动态增加线程的worker机制来解决64 limit。

5. complete_routine
单线程的completion routine(完成例程)。

6. overlap
重叠I/O

7. iocp
IOCP模型(完成端口)实现，线程之间使用message机制来通信。


### tests
一个简单的python测试脚本
一个使用单线程完成端口实现的多连接client，使用了ConnectEx()。


