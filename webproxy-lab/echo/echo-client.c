#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 4096
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT "9000"

static int g_sockfd = -1;

static void die(const char *message)
{
    perror(message);
    exit(1);
}

static void handle_exit_signal(int signo)
{
    (void)signo;

    if (g_sockfd >= 0)
    {
        close(g_sockfd);
    }
    _exit(0);
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
            fprintf(stderr, "서버가 예상보다 일찍 연결을 닫았습니다.\n");
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
        fprintf(stderr, "getaddrinfo 오류: %s\n", gai_strerror(rc));
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
        fprintf(stderr, "%s:%s 에 연결하지 못했습니다.\n", host, port);
        exit(1);
    }

    return sockfd;
}

static bool is_exit_command(const char *buf)
{
    size_t len = strlen(buf);

    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    {
        len--;
    }

    if (len != 4)
    {
        return false;
    }

    return tolower((unsigned char)buf[0]) == 'e' &&
           tolower((unsigned char)buf[1]) == 'x' &&
           tolower((unsigned char)buf[2]) == 'i' &&
           tolower((unsigned char)buf[3]) == 't';
}

int main(int argc, char **argv)
{
    int sockfd;
    char sendbuf[BUF_SIZE];
    char recvbuf[BUF_SIZE];
    const char *host = DEFAULT_HOST;
    const char *port = DEFAULT_PORT;
    bool interactive = false;

    if (argc > 3)
    {
        fprintf(stderr, "사용법: %s [host] [port]\n", argv[0]);
        return 1;
    }

    if (argc >= 2)
    {
        host = argv[1];
    }
    if (argc == 3)
    {
        port = argv[2];
    }

    interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    setvbuf(stdout, NULL, _IOLBF, 0);

    signal(SIGHUP, handle_exit_signal);
    signal(SIGINT, handle_exit_signal);
    signal(SIGTERM, handle_exit_signal);

    sockfd = connect_to_server(host, port);
    g_sockfd = sockfd;
    printf("%s:%s 에 연결되었습니다.\n", host, port);
    printf("한 줄 입력 후 Enter를 누르세요. 종료는 Ctrl+D 또는 exit 입니다.\n");

    while (true)
    {
        if (interactive)
        {
            printf("echo> ");
            fflush(stdout);
        }

        if (fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
        {
            if (interactive)
            {
                printf("\n");
            }
            break;
        }

        size_t len = strlen(sendbuf);
        write_all(sockfd, sendbuf, len);

        if (is_exit_command(sendbuf))
        {
            break;
        }

        read_exact(sockfd, recvbuf, len);
        printf("S -> C : ");
        fwrite(recvbuf, 1, len, stdout);
        if (len > 0 && recvbuf[len - 1] != '\n')
        {
            printf("\n");
        }
    }

    close(sockfd);
    g_sockfd = -1;
    return 0;
}
