/**
 *  @file:   socket.cpp
 *  @brief:  A simple echo server, use BSD socket api
 *
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 */



#include "../common/utility.h"
#include "../common/thread.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>
#include <memory>
#include <functional>




void    handle_client(SOCKET sockfd);



int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: %s $host $port"), argv[0]);
        exit(1);
    }

    const TCHAR* host = argv[1];
    const TCHAR* port = argv[2];

	ADDRINFOT* aiList = NULL;
	ADDRINFOT hints = {};
    hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME;
	int error = GetAddrInfo(host, port, &hints, &aiList);
	if (error != 0)
	{
        LOG_PRINT(_T("getaddrinfo() failed, %s, %s"), host, port);
		return 1;
	}
	
	// loop through the info list, connect the first
    SOCKET socket_listen = INVALID_SOCKET;
	for (ADDRINFOT* pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
	{
		socket_listen = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
		if (socket_listen == INVALID_SOCKET)
		{
			LOG_PRINT(_T("socket() failed"));			
			continue;
		}

		error = bind(socket_listen, pinfo->ai_addr, pinfo->ai_addrlen);
		if (error == SOCKET_ERROR)
		{
			LOG_PRINT(_T("bind() failed, addr: %s, len: %d"), pinfo->ai_addr, pinfo->ai_addrlen);
			closesocket(socket_listen);
			socket_listen = INVALID_SOCKET;
            continue;
		}

        error = listen(socket_listen, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            LOG_PRINT(_T("listen() failed"));
            closesocket(socket_listen);
            continue;
        }

        _tprintf(_T("server listen at %s:%s.\n"), host, port);

        break;
	}

	FreeAddrInfo(aiList);

	if (socket_listen == INVALID_SOCKET)
	{		
		closesocket(socket_listen);		
		return 1;
	}

    std::list<std::shared_ptr<thread>>  thread_list;
    for (;;)
    {
        sockaddr_in addr = {};
        int len = sizeof(addr);
        SOCKET sock_accept = accept(socket_listen, (sockaddr*)&addr, &len);
        if (sock_accept == INVALID_SOCKET)
        {
            LOG_PRINT(_T("listen() failed"));
            break;
        }
        try
        {
            _tstring strAddr = AddressToString(addr);
            _tstring strDate = GetDateTimeString();
            _tprintf(_T("%s, %s connected.\n"), strDate.data(), strAddr.data());
            std::shared_ptr<thread> thrd_ptr(new thread(std::bind(handle_client, sock_accept)));
            thread_list.push_back(thrd_ptr);
        }
        catch (std::bad_alloc&)
        {
            LOG_PRINT(_T("allocate thread object failed"));
        }        
    }

    return 0;
}


void    handle_client(SOCKET sockfd)
{
    char databuf[BUFSIZ];
    for (;;)
    {
        int bytes_read = recv(sockfd, databuf, BUFSIZ, 0);
        if (bytes_read == SOCKET_ERROR)
        {
            LOG_PRINT(_T("recv() failed\n"));
            closesocket(sockfd);
            break;
        }
        else if (bytes_read == 0)
        {
            closesocket(sockfd);
            break;
        }

        // write back what we read
        int bytes_send = send(sockfd, databuf, bytes_read, 0);
        if (bytes_send == SOCKET_ERROR)
        {
            LOG_PRINT(_T("send() failed\n"));
            closesocket(sockfd);
            break;
        }    
    }    
}
