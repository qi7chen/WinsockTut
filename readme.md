# WinsockTut

Simple TCP echo server implement with different Winsock I/O strategies.


### Build    

Obtain [premake4](http://industriousone.com/premake/download).

To build via Visual Studio simply execute:

    premake vs2013
    

### Introduction

I/O Model       | Description
----------------|------------
socket          | basic BSD socket
select          | with select()
async_select    | with WSAAsyncSelect()
async_event     | with WSAAsyncSelect()
complete_routine| with alertable I/O
overlap         | with overlapped I/O
iocp            | with Completion Port, both server and client usage


`test_echo.py` is a simple python script to test server performance
