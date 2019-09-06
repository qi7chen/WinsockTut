# Echo


包含了使用Reactor模型实现的echo server和echo client使用示例


# 使用方式

### IO模型

* 1 [select](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select)
* 2 [WSAAsyncSelect](https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsaasyncselect)
* 3 [WSAEventSelect](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsaeventselect)

### echo server
    echo.exe  server (IP) (端口) (IO模型)

### echo client
    echo.exe client (IP) (端口) (IO模型)
