#include "socket.h"
#include <stdio.h>
#include <WS2tcpip.h>
#include <process.h>
#include "common/utility.h"


/* thread for each connection */
static unsigned CALLBACK handle_client(void* param)
{
    SOCKET sockfd = (SOCKET)param;
    char databuf[kDefaultBufferSize];
    for (;;)
    {
        int bytes = recv(sockfd, databuf, kDefaultBufferSize, 0);
        if (bytes == SOCKET_ERROR)
        {
            fprintf(stderr, "socket %d recv() failed, %s", sockfd, LAST_ERROR_MSG);
            break;
        }
        else if (bytes == 0) /* closed */
        {
            break;
        }
        /* send back */
        bytes = send(sockfd, databuf, bytes, 0);
        if (bytes == SOCKET_ERROR)
        {
            fprintf(stderr, "socket %d send() failed, %s", sockfd, LAST_ERROR_MSG);
            break;
        }    
    }
    closesocket(sockfd);
    fprintf(stdout, "socket %d closed at %s.\n", sockfd, Now());
    return 0;
}

int on_accept(SOCKET sockfd)
{
    fprintf(stderr, "socket %d accepted, %s.\n", sockfd, Now());
    return _beginthreadex(NULL, 0, handle_client, (void*)sockfd, 0, NULL);
}

/* create acceptor socket */
SOCKET  create_acceptor(const char* host, const char* port)
{
    int error;
    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo* aiList = NULL;
    struct addrinfo hints;
    struct addrinfo* pinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(host, port, &hints, &aiList);
    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo() failed, %s:%s, %s.\n", host, port, gai_strerror(error));
        return INVALID_SOCKET;
    }
    for (pinfo = aiList; pinfo != NULL; pinfo = pinfo->ai_next)
    {
        sockfd = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
            fprintf(stderr,"socket() failed, %s", LAST_ERROR_MSG);
            continue;
        }
        error = bind(sockfd, pinfo->ai_addr, pinfo->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, "bind() failed, addr: %s, len: %d, %s", 
                pinfo->ai_addr, pinfo->ai_addrlen, LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        error = listen(sockfd, SOMAXCONN);
        if (error == SOCKET_ERROR)
        {
            fprintf(stderr, "listen() failed, %s", LAST_ERROR_MSG);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }        
        break;
    }
    freeaddrinfo(aiList);
    fprintf(stdout, "server listen at %s:%s\n", host, port);
    return sockfd;
}
