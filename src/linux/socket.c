/**
 *  @file   socket.c
 *  @author ichenq@gmail.com
 *  @date   Apri 25, 2014
 *  @brief  simple echo server, per-thread each connection.
 *          build with `-lpthread` option.
 *
 */
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define BUF_SIZE  512

static int create_acceptor(const char* node, const char* service);
static void* client_handler(void*);


int main(int argc, char* argv[])
{
    int acceptor;
    
    if (argc < 3) {
        printf("Usage: socket [node] [service].\n");
        return 1;
    }
    
    acceptor = create_acceptor(argv[1], argv[2]);
    if (acceptor == -1) {
        return 1;
    }
    fprintf(stdout, "server started at %s:%s.\n", argv[1], argv[2]);
    for (;;) {
        pthread_t tid;
        char straddr[40];
        struct sockaddr_in addr;
        int addr_len = sizeof(addr);
        int fd = accept(acceptor, (struct sockaddr*)&addr, &addr_len);
        if (fd == -1) {
            fprintf(stderr, "accept() failed, %s.\n", strerror(errno));
            break;
        }
        fprintf(stdout, "%s connected.\n", inet_ntop(AF_INET, &addr, straddr, addr_len));
        pthread_create(&tid, NULL, client_handler, (void*)fd);
    }
    close(acceptor);
    return 0;
}

int create_acceptor(const char* node, const char* service) 
{
    int err, fd;
    struct addrinfo* ai_list = NULL;
    struct addrinfo* info = NULL;
    struct addrinfo hints = {};
    
    assert(node && service);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    err = getaddrinfo(node, service, &hints, &ai_list);
    if (err != 0) {
        fprintf(stderr, "%s.\n", gai_strerror(err));
        return -1;
    }
    for (info = ai_list; info != NULL; info = info->ai_next) {
        int yes = 1;
        fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (fd == -1) {
            fprintf(stderr, "socket() failed, %s\n.", strerror(errno));
            continue;
        }
        err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if (err != 0) {
            fprintf(stderr, "setsockopt() failed, %s.\n", strerror(errno));
            close(fd);
            break;
        }
        err = bind(fd, info->ai_addr, info->ai_addrlen);
        if (err != 0) {
            fprintf(stderr, "bind() failed, %s\n.", strerror(errno));
            close(fd);
            break;
        }
        err = listen(fd, SOMAXCONN);
        if (err != 0) {
            fprintf(stderr, "listen() failed, %s.\n", strerror(errno));
            close(fd);
            break;
        }
    }
    freeaddrinfo(ai_list);
    return fd;
}

void* client_handler(void* arg)
{
    int fd = (int)arg;
    char buffer[BUF_SIZE];
    for (;;) {
        int bytes = recv(fd, buffer, BUF_SIZE, 0);
        if (bytes == -1) {
            fprintf(stderr, "recv() failed, %s.\n", strerror(errno));
            break;
        }
        else if (bytes == 0) {
            break; // connection closed
        }
        
        // send message back
        bytes = send(fd, buffer, bytes, 0);
        if (bytes == -1) {
            fprintf(stderr, "recv() failed, %s.\n", strerror(errno));
            break;
        }
    }
    close(fd);
    fprintf(stdout, "socket %d closed.\n", fd);
}


