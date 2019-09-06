# PingPong


包含了使用Reactor模型实现的pingpong server和pingpong client使用示例


# 使用方式

### IO模型

* 1 [Overlapped I/O](https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-overlapped)
* 2 [Alertable IO](https://docs.microsoft.com/en-us/windows/win32/fileio/alertable-i-o)
* 3 [WSAEventSelect](https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports)

### pingpong server
    pingpong.exe  server (IP) (端口) (IO模型)

### pingpong client
    pingpong.exe client (IP) (端口) (IO模型)
