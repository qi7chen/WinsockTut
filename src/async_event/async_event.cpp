/**
*  @brief:  A simple echo server, use threaded WSAAsynSelect mode
*  @author: ichenq@gmail.com
*  @date:   Oct 19, 2011
*/

#include "../common/thread.h"
#include "worker.h"
#include <vector>


using std::tr1::shared_ptr;

static SOCKET  create_listen_socket(const _tstring& strHost, const _tstring& strPort);
static void    add_socket_to_woker(std::vector<shared_ptr<worker>>& workers, SOCKET sockfd);




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
    std::vector<shared_ptr<worker>> workers;

    for (;;)
    {
        SOCKET socknew = accept(sockfd, NULL, NULL);
        if (socknew == INVALID_SOCKET)
        {
            LOG_DEBUG(_T("accept() failed"));
            break;
        }

        add_socket_to_woker(workers, socknew);
    }


    return 0;
}


SOCKET create_listen_socket(const _tstring& strHost, const _tstring& strPort)
{
    sockaddr_in addr = {};
    const _tstring& strAddr = strHost + _T(":") + strPort;
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

    _tprintf(_T("%s, server start listen at %s...\n"), Now().data(), strAddr.data());

    return sockfd;
}


void add_socket_to_woker(std::vector<shared_ptr<worker>>& workers, SOCKET sockfd)
{
    shared_ptr<worker> worker_ptr;
    for (size_t i = 0; i < workers.size(); ++i)
    {
        if (!workers[i]->is_full())
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
        ::Sleep(500);    // wait worker thread been initialized
    }
    
    send_message_to(worker_ptr->get_id(), WM_ADD_NEW_SOCKET, sockfd, 0);
}