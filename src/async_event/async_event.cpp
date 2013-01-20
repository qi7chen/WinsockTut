//  A simple echo server, use threaded WSAAsynSelect mode
//  by ichenq@gmail.com at Oct 19, 2011


#include "worker.h"
#include <vector>

#pragma comment(lib, "ws2_32")


SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort);
void    add_socket_to_woker(std::vector<worker*>& workers, SOCKET sockfd);


// initialize winsock
static global_init init;


// main entry
int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: $program $host $port"));
        return 1;
    }

    SOCKET sockfd = create_listen_socket(argv[1], argv[2]);
    if (sockfd == INVALID_SOCKET)
    {
        return 1;
    }

    // all thread workers
    std::vector<worker*> workers;

    for (;;)
    {
        SOCKET socknew = accept(sockfd, NULL, NULL);
        if (socknew == INVALID_SOCKET)
        {
            _tprintf(_T("accept() failed, %s"), LAST_ERROR_MSG);
            break;
        }

        add_socket_to_woker(workers, socknew);
    }

    // release resouce
    for (size_t i = 0; i < workers.size(); ++i)
    {
        delete workers[i];
    }

    return 0;
}


SOCKET create_listen_socket(const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    const _tstring& strAddr = strHost + _T(":") + strPort;
    if (!StringToAddress(strAddr, &addr))
    {
        _tprintf(_T("StringToAddress() failed (%s), %s"), strAddr.data(), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        _tprintf(_T("socket() failed, %s"), LAST_ERROR_MSG);
        return INVALID_SOCKET;
    }

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        _tprintf(_T("bind() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR)
    {
        _tprintf(_T("listen() failed, %s"), LAST_ERROR_MSG);
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    _tprintf(_T("server start listen [%s] at %s.\n"), strAddr.data(), Now().data());

    return sockfd;
}

// create a new workder if the current one got the 64 limit
void add_socket_to_woker(std::vector<worker*>& workers, SOCKET sockfd)
{
    worker* pWorker = NULL;
    for (size_t i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]->is_full())
        {
            pWorker = workers[i];
            break;
        }
    }

    // create a new one if cannot satisfy
    if (pWorker == NULL)
    {
        pWorker = new worker;
        pWorker->start();
        workers.push_back(pWorker);
        ::Sleep(100);    // wait worker thread create its message queue
    }
    
    if (!send_message_to(pWorker->get_id(), WM_ADD_NEW_SOCKET, sockfd, 0))
    {
        _tprintf(_T("send message to worker thread[%d] failed, %s"), 
            pWorker->get_id(), LAST_ERROR_MSG);
    }
}