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

## 기본 빌드만 할 때

```bash
cd /workspaces/SW_AI-W08-webproxy_lab/webproxy-lab/echo
make
```

## 왜 이렇게 두었나

이전처럼 실행 파일을 고정 이름으로 저장하면, 맥에서 만든 파일을 도커에서 실행할 때 `Exec format error`가 날 수 있습니다.

지금은 실제 바이너리를 환경별 `.build` 아래에 따로 저장하므로, 같은 프로젝트를 맥과 도커에서 번갈아 써도 덜 헷갈립니다.
