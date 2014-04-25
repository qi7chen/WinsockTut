/**
 *  @file   select.c
 *  @author ichenq@gmail.com
 *  @date   Apri 25, 2014
 *  @brief  simple echo server, with `select()`
 *
 */

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE  512
#define SET_SIZE (FD_SETSIZE - 1)

struct connection_set
{
    int     size;
    int     array[SET_SIZE];
};

// connection set operation
static struct connection_set* create_connection_set();
static void destroy_connection_set(struct connection_set* set);
static void add_connection(struct connection_set* set, int fd);
static void del_connection(struct connection_set* set, int fd);

static int create_acceptor(const char* node, const char* service);
static int select_loop(struct connection_set* set, int fd);

// socket event handler
static int on_accept(struct connection_set* set, int fd);
static int on_recv(int fd);
static void on_close(struct connection_set* set, int fd);


// main entry
int main(int argc, char* argv[])
{
    int acceptor;
    if (argc < 3) {
        fprintf(stdout, "Usage: socket [node] [service].\n");
        return 1;
    }
    acceptor = create_acceptor(argv[1], argv[2]);
    if (acceptor == -1) {
        return 1;
    }
    struct connection_set* set = create_connection_set();
    fprintf(stdout, "server started at %s:%s.\n", argv[1], argv[2]);
    
    for (;;) {
        select_loop(set, acceptor);
    }
    
    destroy_connection_set(set);
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

// select() loop
int select_loop(struct connection_set* set, int acceptor)
{
    int i;
    int nready;
    fd_set rdfds = {};
    struct timeval timeout = {0, 50000}; // 50ms
    
    FD_SET(acceptor, &rdfds);
    for(i = 0; i < set->size; ++i) {
        FD_SET(set->array[i], &rdfds);
    }
    
    nready = select(0, &rdfds, NULL, NULL, &timeout);
    if (nready == -1) {
        fprintf(stderr, "select() failed, %s.\n", strerror(errno));
        return -1;
    }
    if (nready == 0) {
        // timed out
    }
    for (i = 0; i < set->size; ++i) {
        int fd = set->array[i];
        if (FD_ISSET(fd, &rdfds)) {
            if (on_recv(fd) == -1) {
                on_close(set, fd);
            }
        }
    }
    
    // new connection comming
    if (FD_ISSET(acceptor, &rdfds)) {
        on_accept(set, acceptor);
    }
    return 0;
}

void on_close(struct connection_set* set, int fd)
{
    close(fd);
    del_connection(set, fd);
}

int on_recv(int fd)
{
    char buffer[BUF_SIZE];
    int bytes = recv(fd, buffer, BUF_SIZE, 0);
    if (bytes == -1) {
        fprintf(stderr, "recv() failed, %s.\n", strerror(errno));
        return -1;
    }
    else if (bytes == 0) {
        return -1; // connection closed
    }
    
    // send message back
    bytes = send(fd, buffer, bytes, 0);
    if (bytes == -1) {
        fprintf(stderr, "recv() failed, %s.\n", strerror(errno));
        return -1;
    }
    return 0;
}

int on_accept(struct connection_set* set, int acceptor)
{
    int err;
    int addrlen;
    int fd;
    struct sockaddr_in addr;
    
    if (set->size == SET_SIZE) {
        fprintf(stderr, "connection full: %d.\n", set->size);
        return -1;
    }
    
    fd = accept(acceptor, (struct sockaddr*)&addr, &addrlen);
    if (fd == -1) {
        fprintf(stderr, "accept() failed, %s.\n", strerror(errno));
        return -1;
    }
    // set non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}

struct connection_set* create_connection_set()
{
    int i;
    struct connection_set* set = NULL;
    set = (struct connection_set*)malloc(sizeof(struct connection_set));
    set->size = 0;
    memcpy(set->array, 0, sizeof(set->array));
    return set;
}

void destroy_connection_set(struct connection_set* set)
{
    free(set);
}

void add_connection(struct connection_set* set, int fd)
{
    int loop = 0;
    int pos = 0;
    
    assert(set->size < SET_SIZE);
    loop = SET_SIZE;
    pos = set->size;
    // find a empty hole
    while (loop--) {
        if (set->array[pos] == 0) {
            set->array[pos] = fd;
            set->size++;
            break;
        }
        pos = (pos + 1) % SET_SIZE;
    }
}

void del_connection(struct connection_set* set, int fd)
{
    int i; 
    for (i = 0; i < set->size; i++) {
        if (set->array[i] == fd) {
            set->array[i] = 0;
            set->size--;
            break;
        }
    }
}

