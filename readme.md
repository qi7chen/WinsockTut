#WinsockExamples
Winsock network programming examples.
Use 7 different I/O strategies to implement an simple echo server.



### build  
[Premake4](http://industriousone.com/premake/download)script

premake4 vs2008

### src
1. socket

Basic BSD socket strategy

2. select

I/O multiplexing with select()

3. async_select
I/O multiplexing with WSAAsyncSelect()

4. async_event

I/O multiplexing with WSAEventSelect(), with a dynamic thread pool for the devil 64 limit.

5. complete_routine

Completion routine and alertable I/O.

6. overlap
I/O multiplexing with overlapped I/O.

7. iocp

Asynchronous sockets with I/O completion port


### tests

a single-thread muliti-client program with I/O completion port and ConnectEx()

a simple python test client



