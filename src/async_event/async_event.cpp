/**
*  @brief:  A simple echo server, use threaded WSAAsynSelect mode
*  @author: ichenq@gmail.com
*  @date:   Oct 19, 2011
*/


#include "worker.h"
#include <vector>


SOCKET  create_server_socket(const _tstring& strAddr);
void    add_to_woker(std::vector<shared_ptr<worker>>& workers, SOCKET sockfd);




int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        return 1;
    }

    _tstring host = argv[1];
    _tstring port = argv[2];

    SOCKET sockfd = create_server_socket(host + _T(":") + port);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    // all thread workers
    std::vector<shared_ptr<worker>> workers;

    for (;;)
    {
        SOCKET socknew = accept(sockfd, NULL, NULL);
        if (socknew == INVALID_SOCKET)
        {
            LOG_DEBUG(_T("accept() failed"));
            break;
        }

        add_to_woker(workers, socknew);
    }


    return 0;
}


SOCKET create_server_socket(const _tstring& strAddr)
{
    sockaddr_in addr = {};
    if (!StringToAddress(strAddr, &addr))
    {
        LOG_PRINT(_T("StringToAddress() failed, %s"), strAddr.data());
        return INVALID_SOCKET;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        LOG_PRINT(_T("socket() failed"));
        return INVALID_SOCKET;
    }

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        LOG_PRINT(_T("bind() failed, %s"), strAddr.data());
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR)
    {
        LOG_PRINT(_T("listen() failed"));
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    return sockfd;
}


void add_to_woker(std::vector<shared_ptr<worker>>& workers, SOCKET sockfd)
{
    shared_ptr<worker> worker_ptr;
    for (size_t i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]->full())
        {
            worker_ptr = workers[i];
            break;
        }
    }

    // create a new one if cannot satisfy
    if (!worker_ptr)
    {
        worker_ptr.reset(new worker);
        worker_ptr->start();
        workers.push_back(worker_ptr);
    }

    worker_ptr->push_socket(sockfd);
}