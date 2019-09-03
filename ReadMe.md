# WinsockTut

尝试用不同的Windows API实现网络I/O多路复用模型
    

### Introduction

I/O Model                |  Source File                | Description
------------------------ |-----------------------------|------------
select() poller          |  src/SelectPoller.cpp       | select() API
WSAAsyncSelect() poller  |  src/AsyncSelectPoller.cpp  | WSAAsyncSelect() API
WSAEventSelect() poller  |  src/AsyncEventPoller.cpp   | WSAEventSelect() API
alertable I/O            |                             | alertable I/O
overlapped I/O           |                             | overlapped I/O
I/O Completion Port      |                             | I/O Completion Port


### Example 

[echo示例](https://github.com/ichenq/WinsockTut/tree/master/examples/echo)
