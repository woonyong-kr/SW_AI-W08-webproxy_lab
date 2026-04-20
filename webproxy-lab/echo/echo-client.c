#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

static void read_exact(int fd, char *buf, size_t len)
{
    size_t received = 0;

    while (received < len)
    {
        ssize_t rc = read(fd, buf + received, len - received);
        if (rc < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            die("read");
        }
        if (rc == 0)
        {
            fprintf(stderr, "server closed connection early\n");
            exit(1);
        }
        received += (size_t)rc;
    }
}

static int connect_to_server(const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;
    int sockfd = -1;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        exit(1);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd < 0)
        {
            continue;
        }

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(result);

    if (sockfd < 0)
    {
        fprintf(stderr, "failed to connect to %s:%s\n", host, port);
        exit(1);
    }

    return sockfd;
}

int main(int argc, char **argv)
{
    int sockfd;
    char sendbuf[BUF_SIZE];
    char recvbuf[BUF_SIZE];

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    sockfd = connect_to_server(argv[1], argv[2]);
    printf("connected to %s:%s\n", argv[1], argv[2]);
    printf("enter a line and press Enter. Ctrl+D to quit.\n");

    while (true)
    {
        if (fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
        {
            break;
        }

        size_t len = strlen(sendbuf);
        write_all(sockfd, sendbuf, len);
        read_exact(sockfd, recvbuf, len);
        fwrite(recvbuf, 1, len, stdout);
    }

    close(sockfd);
    return 0;
}
