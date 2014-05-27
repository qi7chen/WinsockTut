#WinsockExamples
Winsock programming examples with 7 different I/O strategies

### Build
    * download [premake4](http://industriousone.com/premake/download)
    * premake4 vs2008

### Directory
    * src
        + socket, BSD socket
        + select, I/O multiplexing with select()
        + async_select, I/O multiplexing with WSAAsyncSelect()
        + async_event, I/O multiplexing with WSAEventSelect()
        + complete_routine, I/O multiplexing with alertable I/O
        + overlap, I/O multiplexing with overlapped I/O
        + iocp, I/O multiplexing with Completion Port

    * tests
        + test_tcp_client.py, max 512 client connections to test server performance
        + test_client, client connections multiplexed by I/O Completion Port 




