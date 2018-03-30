# WinsockTut

尝试用不同的Windows API实现网络I/O多路复用模型
    

### Introduction

I/O Model         | Description
------------------|------------
SelectPoller      | select() API
AsyncSelectPoller | WSAAsyncSelect() API
AsyncEventPoller  | WSAEventSelect() API
complete_routine  | alertable I/O
overlap           | overlapped I/O
iocp              | Completion Port, both server and client usage


### Example 

[echo示例](https://github.com/ichenq/WinsockTut/tree/master/examples/echo)
