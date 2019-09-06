# 测试用例

使用[golang](https://golang.org/)实现的echo和pingpong测试用例


# 如何使用

* 下载并安装Golang环境https://golang.org/dl/
* 执行对应的go命令
 
 
## 测试过程

### 开启client测试server
    
    执行命令 `go run echo.go -m=server -h=:8081 -e5`

### 开启server测试client

    执行命令 `go run echo.go -m=client -h=127.0.0.1:8081 -c32 -e5`
    
### 命令行解释

* -m 服务器或者客户端模式
* -h 服务器IP地址和端口
* -c 并发客户端连接数量
* -e 每个客户端发送多少个包后停止


## Why Go

* python和nodejs都可以做这个测试；
* Go标准库已经集成了优秀的网络库实现，无需导入第三方库(twisted, netty etc)；
* goroutine用来模拟多个客户端连接很直接；
