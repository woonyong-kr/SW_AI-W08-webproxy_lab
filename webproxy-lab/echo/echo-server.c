#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 16
#define BUF_SIZE 4096

static void die(const char *message)
{
    perror(message);
    exit(1);
}

static void write_all(int fd, const char *buf, size_t len)
{
    size_t written = 0;

    while (written < len)
    {
        ssize_t rc = write(fd, buf + written, len - written);
        if (rc < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            die("write");
        }
        written += (size_t)rc;
    }
}

static void echo_client(int connfd)
{
    char buf[BUF_SIZE];

    while (true)
    {
        ssize_t n = read(connfd, buf, sizeof(buf));
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            die("read");
        }
        if (n == 0)
        {
            return;
        }
        write_all(connfd, buf, (size_t)n);
    }
}

int main(int argc, char **argv)
{
    int listenfd;
    struct sockaddr_in server_addr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        die("socket");
    }

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) < 0)
    {
        die("setsockopt");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        die("bind");
    }

    if (listen(listenfd, BACKLOG) < 0)
    {
        die("listen");
    }

    printf("echo-server listening on port %s\n", argv[1]);

    while (true)
    {
        int connfd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        char client_ip[INET_ADDRSTRLEN];

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            die("accept");
        }

        if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
                      sizeof(client_ip)) == NULL)
        {
            strncpy(client_ip, "unknown", sizeof(client_ip));
            client_ip[sizeof(client_ip) - 1] = '\0';
        }

        printf("client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        echo_client(connfd);
        close(connfd);
        printf("client disconnected: %s:%d\n", client_ip,
               ntohs(client_addr.sin_port));
    }
}
