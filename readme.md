#WinsockExamples
Winsock套接字编程示例，7种不同的I/O复用策略

### 构建 
    * premake4 vs2008

### 目录
    * src
        + socket， BSD套接字
        + select， 使用select()来实现I/O复用
        + async_select， 使用WSAAsyncSelect()来实现I/O复用
        + async_event，使用WSAEventSelect()来实现I/O复用
        + complete_routine， 完成例程和可提醒I/O
        + overlap，使用重叠I/O来实现I/O复用
        + iocp，多线程的I/O完成端口来实现I/O复用

    * tests
        + test_case.py，使用select()管理多个客户端连接的Python脚本
        + test_client，单线程的I/O完成端口事件循环




