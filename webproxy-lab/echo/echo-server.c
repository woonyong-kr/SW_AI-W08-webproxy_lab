#include <arpa/inet.h>
#include <ctype.h>
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

#define BACKLOG 16  // 아직 accept되지 않은 연결 요청 대기열 크기 힌트
#define BUF_SIZE 4096
#define DEFAULT_PORT "9000"

static void die(const char* message) {
  perror(message);
  exit(1);
}

static void echo_write(int fd, const char* buf, size_t len) {
  size_t written = 0;

  while (written < len) {
    ssize_t rc = write(fd, buf + written, len - written);
    if (rc < 0) {
      if (errno == EINTR) {
        continue;
      }
      die("write");
    }
    written += (size_t)rc;
  }
}

static char* echo_read(int fd, char* buf, size_t buf_size) {
  size_t received = 0;

  if (buf_size == 0) {
    return NULL;
  }

  while (true) {
    char ch;
    ssize_t n = read(fd, &ch, 1);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      die("read");
    }

    if (n == 0) {
      if (received == 0) {
        return NULL;
      }
      break;
    }

    buf[received] = ch;
    received++;

    if (ch == '\n' || received == buf_size - 1) {
      break;
    }
  }

  buf[received] = '\0';
  return buf;
}

static bool is_exit_command(const char* buf) {
  size_t len = strlen(buf);

  while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
    len--;
  }

  if (len != 4) {
    return false;
  }

  return tolower((unsigned char)buf[0]) == 'e' &&
         tolower((unsigned char)buf[1]) == 'x' &&
         tolower((unsigned char)buf[2]) == 'i' &&
         tolower((unsigned char)buf[3]) == 't';
}

static void handle_client(int connfd, const char* client_ip,
                          unsigned short client_port) {
  char buf_storage[BUF_SIZE];
  char* buf = buf_storage;

  while ((buf = echo_read(connfd, buf, sizeof(buf_storage))) != NULL) {
    if (is_exit_command(buf)) {
      break;
    }

    printf("[child %ld] C -> S : %s", (long)getpid(), buf);
    echo_write(connfd, buf, strlen(buf));
  }

  close(connfd);
  printf("[child %ld] 해제: %s:%u\n", (long)getpid(), client_ip, client_port);
}

int main(int argc, char** argv) {
  // argv[0] == "./echo-server"
  // argv[1] == "9000"  // 선택 사항

  int listenfd;
  struct sockaddr_in server_addr;
  const char* port = DEFAULT_PORT;

  // 포트를 생략하면 기본값 9000 사용
  // 인자를 너무 많이 주는 경우만 에러 처리
  if (argc > 2) {
    fprintf(stderr, "사용법: %s [port]\n", argv[0]);
    return 1;
  }
  if (argc == 2) {
    port = argv[1];
  }

  setvbuf(stdout, NULL, _IOLBF, 0);

  // signal(시그널종류, 처리방식);
  //
  // 첫 번째 인자:
  // SIGPIPE: 끊어진 연결에 쓰기 시도할 때 나는 신호
  // SIGINT : Ctrl + C 입력 시 오는 신호
  // SIGTERM: 프로세스 종료 요청 신호
  // SIGQUIT: Ctrl + \ 입력 시 오는 신호
  // SIGHUP : 터미널 연결 종료 등에 쓰이는 신호
  // SIGALRM: alarm() 타이머 만료 신호
  // SIGCHLD: 자식 프로세스 상태가 바뀔 때 오는 신호
  // SIGUSR1: 사용자 정의 신호 1
  // SIGUSR2: 사용자 정의 신호 2
  // SIGABRT: abort() 호출 시 나는 신호
  // SIGSEGV: 잘못된 메모리 접근 시 나는 신호
  // SIGFPE : 0으로 나누기 같은 산술 예외 신호
  // SIGILL : 잘못된 명령어 실행 시 나는 신호
  //
  // 참고:
  // SIGKILL, SIGSTOP도 시그널이지만 signal()로 무시하거나
  // 사용자 정의 handler를 등록할 수는 없다.
  //
  // 두 번째 인자:
  // SIG_IGN : 해당 신호를 무시
  // SIG_DFL : 운영체제 기본 동작으로 처리
  // handler : 직접 만든 함수로 처리
  //
  // 여기서는 클라이언트가 먼저 연결을 끊어도 서버가 SIGPIPE 때문에
  // 종료되지 않게 하려고 SIGPIPE를 무시한다.

  signal(SIGPIPE, SIG_IGN);
  // fork()로 만든 자식 프로세스가 끝났을 때 좀비 프로세스가 남지 않도록
  // SIGCHLD를 무시해서 커널이 종료 상태를 자동 회수하게 한다.
  signal(SIGCHLD, SIG_IGN);

  // socket(주소체계, 소켓종류, 프로토콜)
  //
  // 첫 번째 인자 address family:
  // AF_INET : IPv4 인터넷 주소 사용
  // AF_INET6: IPv6 인터넷 주소 사용
  // AF_UNIX / AF_LOCAL: 같은 머신 내부 프로세스끼리 통신하는
  //                     유닉스 도메인 소켓 주소 사용
  //
  // 두 번째 인자 socket type:
  // SOCK_STREAM: 연결 지향 바이트 스트림 소켓, 보통 TCP
  // SOCK_DGRAM : 비연결 데이터그램 소켓, 보통 UDP
  //
  // 세 번째 인자 protocol:
  // 0: 앞의 address family + socket type 조합에 맞는
  //    기본 프로토콜을 자동 선택
  //    여기서는 AF_INET + SOCK_STREAM 이므로 보통 TCP

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    die("socket");
  }

  // setsockopt(대상소켓, 옵션레벨, 옵션이름, 옵션값주소, 옵션값크기)
  //
  // 첫 번째 인자 socket fd:
  // listenfd: 옵션을 적용할 대상 소켓
  //
  // 두 번째 인자 option level:
  // SOL_SOCKET : 소켓 공통 레벨 옵션
  // IPPROTO_TCP: TCP 레벨 옵션
  // IPPROTO_IP : IPv4 레벨 옵션
  //
  // 세 번째 인자 option name:
  // SO_REUSEADDR: 같은 주소/포트에 다시 bind하기 쉽게 하는 옵션
  // SO_KEEPALIVE: 연결이 오래 살아 있을 때 상태 확인 패킷 사용
  // SO_REUSEPORT: 같은 포트를 여러 소켓이 나눠 쓰게 하는 옵션
  //
  // 네 번째 인자 option value:
  // &optval: 옵션 값이 저장된 메모리 주소
  // 보통 1이면 옵션 켜기, 0이면 옵션 끄기
  //
  // 다섯 번째 인자 option length:
  // sizeof(optval): 옵션 값의 크기
  //
  // 여기서는 SO_REUSEADDR를 1로 켜서 서버를 빠르게 재시작할 때
  // 같은 포트에 다시 bind하기 쉽게 만든다.
  int optval = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    die("setsockopt");
  }

  // struct sockaddr_in
  // IPv4 소켓 주소 정보를 담는 구조체
  //
  // 이 블록은 통신 타입을 정하는 부분이 아니라
  // "이 서버 소켓을 어떤 주소(IP)와 포트에 bind할지"를 채우는 부분이다.
  //
  // memset(시작주소, 채울값, 바이트수)
  // &server_addr           : 구조체 시작 주소
  // 0                      : 모든 바이트를 0으로 초기화
  // sizeof(server_addr)    : 구조체 전체 크기
  //
  // 먼저 구조체 전체를 0으로 비운 뒤 필요한 필드만 채운다.
  memset(&server_addr, 0, sizeof(server_addr));
  //
  // sin_family:
  // 주소 체계(address family)
  // AF_INET이면 이 주소 구조체가 IPv4 주소를 사용한다는 뜻
  server_addr.sin_family = AF_INET;
  //
  // sin_addr.s_addr:
  // bind할 IP 주소
  // INADDR_ANY는 "이 머신의 모든 네트워크 인터페이스에서 요청을 받겠다"는 뜻
  // htonl()은 host byte order -> network byte order로 바꾼다.
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  //
  // sin_port:
  // bind할 포트 번호
  // port 문자열을 atoi()로 숫자로 바꾸고
  // htons()로 host byte order -> network byte order로 바꾼다.
  server_addr.sin_port = htons((unsigned short)atoi(port));

  if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    die("bind");
  }

  // listen(대상소켓, backlog)
  //
  // 첫 번째 인자 socket fd:
  // listenfd: bind까지 끝난 소켓을 "연결 대기용 소켓"으로 전환
  //
  // 두 번째 인자 backlog:
  // 아직 accept()되지 않았지만 커널 큐에서 대기 중인
  // 연결 요청 개수의 힌트 값
  // 값이 너무 작으면 동시에 접속이 몰릴 때 일부 요청이 거절될 수 있다.
  //
  // 참고:
  // 서버는 listen()으로 수동 대기(passive open) 상태가 되고
  // 클라이언트는 connect()로 능동 연결(active open)을 시도한다.
  if (listen(listenfd, BACKLOG) < 0) {
    die("listen");
  }

  while (true) {
    int connfd;
    pid_t pid;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];
    unsigned short client_port;

    // accept(리슨소켓, 클라이언트주소버퍼, 주소길이버퍼)
    //
    // 첫 번째 인자 socket fd:
    // listenfd: 이미 listen() 상태인 "연결 대기용 소켓"
    // 이 소켓 자체로 데이터를 주고받는 것이 아니라
    // 새 연결 요청을 받아들이는 창구 역할을 한다.
    //
    // 두 번째 인자 client address:
    // (struct sockaddr*)&client_addr
    // 커널이 "어느 클라이언트가 접속했는지" 주소 정보를 써 넣을 버퍼
    // 실제 타입은 struct sockaddr_in(IPv4)이지만
    // accept() 원형이 struct sockaddr*를 받기 때문에 캐스팅해서 넘긴다.
    //
    // 세 번째 인자 client address length:
    // &client_len
    // 호출 전에는: client_addr 버퍼의 크기
    // 호출 후에는: 커널이 실제로 채운 주소 정보의 크기
    // 그래서 sizeof(client_addr) 같은 임시값이 아니라
    // 주소를 넘길 수 있는 변수로 준비해 두어야 한다.
    //
    // 반환값:
    // 성공하면 connfd라는 "새 연결 전용 소켓 fd"를 돌려준다.
    // 이 connfd로 해당 클라이언트와 read()/write()를 수행한다.
    // listenfd는 계속 살아 있으면서 다음 연결을 또 accept()하는 데 사용된다.
    // 실패하면 -1을 반환한다.
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
    if (connfd < 0) {
      if (errno == EINTR) {
        continue;
      }
      die("accept");
    }

    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
                  sizeof(client_ip)) == NULL) {
      strncpy(client_ip, "unknown", sizeof(client_ip));
      client_ip[sizeof(client_ip) - 1] = '\0';
    }
    client_port = ntohs(client_addr.sin_port);

    // fork() 이후 부모/자식이 둘 다 listenfd와 connfd를 가진 상태로 시작한다.
    // 부모는 "다음 accept()"를 계속해야 하므로 connfd를 닫고,
    // 자식은 "이 클라이언트 처리"만 하면 되므로 listenfd를 닫는다.
    pid = fork();
    if (pid < 0) {
      close(connfd);
      die("fork");
    }

    if (pid == 0) {
      close(listenfd);
      printf("[child %ld] 연결: %s:%u (parent=%ld)\n", (long)getpid(),
             client_ip, client_port, (long)getppid());
      handle_client(connfd, client_ip, client_port);
      exit(0);
    }

    printf("[parent %ld] child %ld 생성: %s:%u\n", (long)getpid(), (long)pid,
           client_ip, client_port);
    close(connfd);
  }
}
