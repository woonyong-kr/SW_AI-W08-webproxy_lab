# Echo 연습 폴더

이 폴더는 `webproxy-lab` 안에서 소켓 기본 흐름만 짧게 연습하기 위한 공간입니다.

가장 중요한 점은 바이너리를 현재 환경별로 분리해서 저장한다는 것입니다.

- macOS에서 빌드하면 `echo/.build/Darwin-arm64/...`
- Docker Linux에서 빌드하면 `echo/.build/Linux-x86_64/...`

그래서 맥에서 만든 실행 파일과 도커에서 만든 실행 파일이 서로 덮어쓰지 않습니다.

## 파일 구성

- `echo-server.c`
  - 에코 서버 소스
- `echo-client.c`
  - 에코 클라이언트 소스
- `Makefile`
  - 플랫폼별 빌드 결과를 `.build/<OS>-<ARCH>/`에 저장

## 컨테이너에서 실행

```bash
cd /workspaces/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make run-server PORT=9000
```

다른 터미널:

```bash
cd /workspaces/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make run-client HOST=127.0.0.1 PORT=9000 MESSAGE="hello echo"
```

클라이언트는 이제 인자 없이 실행해도 기본값 `127.0.0.1:9000`으로 연결됩니다.

## 외부 macOS 터미널에서 실행

호스트 macOS 터미널에서 아래처럼 실행하면, VS Code 내장 터미널이 아니라 `Terminal.app` 새 창에서 실행됩니다.

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-server
```

다른 외부 터미널 창에서 클라이언트를 열려면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-client
```

서버와 클라이언트를 한 번에 각각 새 창으로 열려면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-both
```

클라이언트를 여러 개 한 번에 열고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-client 5
```

위 명령은 클라이언트 터미널 5개를 엽니다.

서버 1개와 클라이언트 여러 개를 한 번에 열고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-both 5
```

실시간 모니터링 창을 따로 열고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-monitor
```

이 명령들은 실행 중인 첫 번째 Docker 컨테이너를 자동으로 찾아서 그 안에서 `echo` 예제를 실행합니다. 필요하면 `CONTAINER=<이름>`으로 직접 지정할 수 있습니다.

## 부모/자식 프로세스 확인

`fork()` 기반 병렬 서버로 바뀌었기 때문에:

- 부모 프로세스는 `accept()`만 계속 담당
- 자식 프로세스는 클라이언트 하나를 전담 처리

현재 떠 있는 부모/자식 `echo-server` 프로세스를 한 번만 보고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make docker-ps-server
```

예를 들어 출력에서:

- `PID`는 현재 프로세스 ID
- `PPID`는 부모 프로세스 ID

를 뜻합니다. 자식 프로세스의 `PPID`가 부모 서버의 `PID`와 같으면 그 프로세스가 부모 아래에서 fork된 자식입니다.

서버 로그에도 PID가 찍히도록 되어 있습니다.

예시:

```text
[parent 1234] child 1235 생성: 127.0.0.1:51000
[child 1235] 연결: 127.0.0.1:51000 (parent=1234)
[child 1235] C -> S : hello
[child 1235] 해제: 127.0.0.1:51000
```

## 특정 클라이언트만 찾기

특정 클라이언트를 처리 중인 child 프로세스만 찾고 싶다면, 서버 로그에 찍힌 클라이언트 포트를 사용합니다.

예를 들어 서버 로그에 `127.0.0.1:51000`이 보였다면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make docker-find-client CLIENT_PORT=51000
```

그러면 해당 클라이언트 포트와 연결된 소켓 정보와 PID를 확인할 수 있습니다.

## 실시간 모니터링

현재 `echo-server` 부모/자식 프로세스와 포트 연결 상태를 현재 터미널에서 계속 보고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make docker-monitor-server
```

또는 외부 macOS 터미널 새 창으로 보고 싶으면:

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make open-monitor
```

이 모니터는 주기적으로 아래 정보를 보여줍니다.

- `echo-server.bin` 부모/자식 프로세스 목록
- 해당 포트의 연결 상태
- 어느 PID가 어떤 연결을 잡고 있는지

## 기본 빌드만 할 때

```bash
cd /workspaces/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make
```

## 왜 이렇게 두었나

이전처럼 실행 파일을 고정 이름으로 저장하면, 맥에서 만든 파일을 도커에서 실행할 때 `Exec format error`가 날 수 있습니다.

지금은 실제 바이너리를 환경별 `.build` 아래에 따로 저장하므로, 같은 프로젝트를 맥과 도커에서 번갈아 써도 덜 헷갈립니다.
