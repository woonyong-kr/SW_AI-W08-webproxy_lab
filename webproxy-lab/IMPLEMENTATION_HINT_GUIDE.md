# Web Proxy Lab 구현 힌트 가이드

이 문서는 `week8_issues_complete.csv`의 순서를 기준으로, 이 저장소에서 실제로 무엇부터 어디에 구현해야 하는지 정리한 실전용 가이드입니다.

중요한 원칙은 두 가지입니다.

- 이 문서는 답안을 직접 주지 않습니다.
- 대신 "어느 파일의 어느 함수 자리에 무엇을 채워야 하는지"와 "검증은 어떻게 하는지"만 힌트로 제공합니다.

`ASSIGNMENT_GUIDE.md`가 과제 개요 문서라면, 이 문서는 실제 구현 순서 문서라고 생각하면 됩니다.

## 0. 먼저 알아둘 점

CSV에는 세 종류의 항목이 섞여 있습니다.

1. 개념 학습 항목
2. 책/실습용 예제 구현 항목
3. 이 저장소에서 실제로 수정할 항목

즉, CSV를 위에서 아래로 읽되, 모든 줄이 곧바로 이 저장소의 파일 수정으로 이어지는 것은 아닙니다.

현재 저장소에서 실제로 손대야 할 핵심 파일은 아래 세 개입니다.

- [tiny.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/tiny.c)
- [adder.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/cgi-bin/adder.c)
- [proxy.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/proxy.c)

그리고 구현에 참고할 도우미 코드는 아래에 있습니다.

- [tiny/csapp.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/csapp.c)
- [csapp.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/csapp.c)

## 1. CSV 항목을 실제 작업으로 번역하기

### 공통 항목

아래 항목은 구현 파일 수정이 아니라 개인/팀 정리용입니다.

- 핵심 역량 목표 수립
- 달성률 평가
- AI 활용 원칙 수립
- 이번 주 할 일 검토
- WIL 작성
- 협업 룰 정리

이 항목들은 코드 작업과 분리해서 짧게 메모만 남기고 넘어가면 됩니다.

### 학습 항목

아래 항목들은 "코드를 바로 치기 전에 이해해야 하는 개념"입니다.

- 네트워크 개념과 TCP/IP 계층
- 소켓과 소켓 인터페이스
- 클라이언트-서버 모델
- 파일 디스크립터
- Datagram vs Stream Socket
- 웹서버
- MIME type / 정적 / 동적 / CGI
- HTTP 요청/응답 / 헤더 / 메소드 / 상태코드 / HEAD
- 프록시 서버

이 단계에서는 새 파일을 만들기보다 다음을 보는 것이 좋습니다.

- `Open_listenfd`, `Open_clientfd`가 어떤 역할인지 읽기
- `rio_readlineb`, `rio_readnb`, `rio_writen`이 언제 쓰이는지 이해하기
- HTTP 요청 한 줄이 어떻게 생기는지 예시를 눈으로 익히기

참고로 `open_clientfd`와 `open_listenfd`는 아래 위치에서 읽을 수 있습니다.

- [tiny/csapp.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/csapp.c:951)
- [tiny/csapp.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/csapp.c:998)

## 2. echo 서버 단계는 이 저장소 밖에서 짧게 연습

CSV의 아래 항목은 이해를 위한 선행 연습입니다.

- `open_clientfd()`
- `open_listenfd()`
- 클라이언트 main()
- 서버 main()
- 서버 echo()

중요한 점은, 이 저장소에는 echo 서버 파일이 따로 없다는 것입니다.

즉, echo 단계는 두 가지 중 하나로 처리하면 됩니다.

1. 책의 예제를 읽고 흐름만 이해한 뒤 넘어가기
2. 별도 scratch 파일로 아주 작은 echo client/server를 잠깐 만들어 보기

이 단계에서 얻어야 할 감각은 딱 아래 세 가지입니다.

- 서버는 `listen -> accept -> read -> write -> close` 순서로 돈다
- 클라이언트는 `connect -> write -> read -> close` 순서로 돈다
- 텍스트 기반 프로토콜은 "줄 단위로 읽는 것"이 자주 중요하다

이걸 이해했다면 바로 Tiny 구현으로 넘어가면 됩니다.

## 3. Tiny 서버 구현 순서

이제부터는 실제로 이 저장소 파일을 수정하는 단계입니다.

### 3-1. 첫 번째 목표

처음 목표는 이것 하나입니다.

- 브라우저에서 `http://localhost:<포트>/home.html` 요청 시 Tiny가 정적 파일을 돌려준다

그 다음 목표는 이것입니다.

- 브라우저에서 `http://localhost:<포트>/cgi-bin/adder?123&456` 요청 시 동적 응답이 나온다

### 3-2. 수정 파일과 순서

수정 우선순위는 아래 순서가 가장 좋습니다.

1. `tiny/tiny.c`의 `doit()`
2. `tiny/tiny.c`의 `read_requesthdrs()`
3. `tiny/tiny.c`의 `parse_uri()`
4. `tiny/tiny.c`의 `get_filetype()`
5. `tiny/tiny.c`의 `serve_static()`
6. `tiny/tiny.c`의 `clienterror()`
7. `tiny/tiny.c`의 `serve_dynamic()`
8. `tiny/cgi-bin/adder.c` 확인 및 테스트

중요한 이유는 이 순서가 "요청을 읽고 -> 분기하고 -> 정적 먼저 성공시키고 -> 마지막에 CGI" 흐름과 정확히 맞기 때문입니다.

### 3-3. tiny.c에서 어디를 채워야 하나

현재 [tiny.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/tiny.c:11)에는 함수 선언만 있고, `main()`만 있습니다.

즉, 여러분이 직접 채워야 할 함수는 아래입니다.

- `doit`
- `read_requesthdrs`
- `parse_uri`
- `serve_static`
- `get_filetype`
- `serve_dynamic`
- `clienterror`

구현 위치는 가장 단순하게 `main()` 아래에 위 순서대로 추가하면 됩니다.

### 3-4. 함수별 힌트

#### `doit(int fd)`

역할:
- 한 개의 HTTP 요청 전체를 처리하는 중심 함수

이 함수에서 해야 할 일:
- 요청 라인 읽기
- 메소드 확인
- GET이 아니면 에러 처리
- 헤더 읽기
- URI를 정적/동적으로 분기
- 파일 존재 여부와 권한 검사
- 정적이면 `serve_static`, 동적이면 `serve_dynamic` 호출

힌트:
- 가장 먼저 요청 라인의 세 요소를 분리해 보세요.
- `method`, `uri`, `version` 세 변수로 생각하면 정리가 쉽습니다.
- 분기 전에 "이 URI가 정적인가 동적인가"를 먼저 결정해야 뒤 코드가 깔끔해집니다.

막히는 포인트:
- 헤더를 안 읽고 바로 다음 요청으로 넘어가면 입력 버퍼가 꼬일 수 있습니다.
- 메소드 체크를 늦게 하면 에러 처리가 지저분해집니다.

완료 기준:
- `GET /home.html HTTP/1.1` 요청을 받고, 정적 파일 경로로 분기할 수 있다

#### `read_requesthdrs(rio_t *rp)`

역할:
- 요청 헤더를 빈 줄까지 읽고 버리는 함수

이 함수에서 해야 할 일:
- 한 줄씩 읽기
- `\r\n` 한 줄만 남은 지점까지 반복

힌트:
- 이 함수는 "분석"보다는 "소비"에 가깝습니다.
- 처음에는 헤더 내용을 화면에 찍어 보면서 확인하면 이해가 빨라집니다.

완료 기준:
- 브라우저 요청 시 헤더 여러 줄을 읽고 마지막 빈 줄에서 멈춘다

#### `parse_uri(char *uri, char *filename, char *cgiargs)`

역할:
- URI가 정적 콘텐츠인지 동적 콘텐츠인지 판단
- 로컬 파일 경로와 CGI 인자를 분리

이 함수에서 해야 할 일:
- URI 안에 `cgi-bin`이 있는지 없는지로 크게 나누기
- 정적이면 파일 경로 구성
- 동적이면 `?` 뒤를 인자로 분리

힌트:
- 반환값을 "정적/동적 여부"로 사용하면 뒤 분기가 쉬워집니다.
- `filename`과 `cgiargs`를 둘 다 채운다고 생각하면 헷갈리지 않습니다.

막히는 포인트:
- `/` 요청일 때 기본 파일을 무엇으로 볼지 놓치기 쉽습니다.
- `?`를 나누는 순간 원본 문자열이 바뀐다는 점을 의식해야 합니다.

완료 기준:
- `/home.html`과 `/cgi-bin/adder?123&456`를 서로 다르게 분해할 수 있다

#### `get_filetype(char *filename, char *filetype)`

역할:
- 파일 확장자에 따라 MIME type 문자열을 채움

이 함수에서 해야 할 일:
- `.html`, `.gif`, `.jpg`, `.png`, 일반 텍스트 등을 구분

힌트:
- 처음에는 이 저장소에 실제로 있는 파일 타입만 처리해도 충분합니다.
- `home.html`, `godzilla.gif`, `godzilla.jpg`가 바로 테스트 케이스입니다.

완료 기준:
- `Content-type` 헤더에 올바른 값이 들어간다

#### `serve_static(int fd, char *filename, int filesize)`

역할:
- 정적 파일을 읽어 HTTP 응답으로 보냄

이 함수에서 해야 할 일:
- 응답 헤더 작성
- 파일 열기
- 파일 내용을 클라이언트에 쓰기

힌트:
- 텍스트 파일과 이미지 파일을 같은 흐름으로 보내려면 "문자열"이 아니라 "바이트"로 생각해야 합니다.
- 먼저 헤더가 맞는지 확인하고, 그 다음 본문 전송을 붙이는 순서가 디버깅하기 쉽습니다.

막히는 포인트:
- 파일 본문을 문자열 함수처럼 다루면 바이너리 파일이 깨질 수 있습니다.
- `Content-length`가 빠지면 브라우저 반응이 이상할 수 있습니다.

완료 기준:
- 브라우저에서 `home.html`이 뜨고, 이미지도 깨지지 않는다

#### `clienterror(...)`

역할:
- 오류 상황에서 HTML 본문을 포함한 HTTP 에러 응답 생성

이 함수에서 해야 할 일:
- 상태 줄 만들기
- 간단한 에러 HTML 본문 만들기
- 헤더와 본문을 순서대로 전송

힌트:
- "무슨 에러인지"와 "왜 에러인지"를 분리해서 문자열을 조립하면 깔끔합니다.
- 처음엔 404, 403, 501 정도만 제대로 나오게 해도 충분합니다.

완료 기준:
- 없는 파일 요청 시 브라우저에 에러 페이지가 보인다

#### `serve_dynamic(int fd, char *filename, char *cgiargs)`

역할:
- CGI 프로그램을 실행해서 동적 응답을 생성

이 함수에서 해야 할 일:
- 기본 응답 헤더 일부 작성
- 자식 프로세스 생성
- 환경 변수에 `QUERY_STRING` 전달
- 표준출력을 클라이언트 소켓으로 연결
- CGI 실행

힌트:
- 이 함수는 "서버가 직접 계산"하는 것이 아니라 "다른 프로그램을 대신 실행"하는 흐름입니다.
- `fork`, `setenv`, `dup2`, `execve` 순서를 그림으로 한 번 써보면 바로 정리됩니다.

막히는 포인트:
- 부모/자식 프로세스 역할을 섞으면 디버깅이 어려워집니다.
- CGI 프로그램이 stdout으로 무엇을 내보내는지 꼭 의식해야 합니다.

완료 기준:
- `/cgi-bin/adder?...` 요청 시 브라우저에 동적 결과가 나온다

### 3-5. adder.c는 어떻게 다루면 되나

현재 [adder.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/tiny/cgi-bin/adder.c:7)는 이미 예제 흐름이 들어 있는 상태입니다.

이 파일에서는 "새로 답을 짜기"보다 아래를 확인하는 쪽이 좋습니다.

- `QUERY_STRING`을 어떻게 읽는지
- 두 숫자를 어떻게 분리하는지
- HTTP 헤더를 CGI가 직접 출력한다는 점

즉, 이 파일은 구현 대상이자 동시에 `serve_dynamic()`를 이해하기 위한 참고 예제입니다.

## 4. Tiny 서버 테스트 순서

CSV 테스트 순서를 그대로 적용하면 아래처럼 하면 됩니다.

### 테스트 1. telnet으로 HTTP 버전 확인

목표:
- Tiny가 최소한 HTTP 응답 형식을 갖춘다는 것 확인

추천 방식:
- 서버 실행
- `telnet localhost <포트>`
- `GET /home.html HTTP/1.0` 요청을 직접 입력

볼 포인트:
- 응답 상태 줄
- `Server`, `Connection`, `Content-length`, `Content-type` 같은 헤더

### 테스트 2. 브라우저에서 home.html 확인

목표:
- 정적 콘텐츠 흐름 검증

주소:
- `http://localhost:<포트>/home.html`

### 테스트 3. 브라우저에서 adder 확인

목표:
- 동적 콘텐츠 흐름 검증

주소 예시:
- `http://localhost:<포트>/cgi-bin/adder?123&456`

볼 포인트:
- 쿼리 문자열이 CGI까지 전달되는가
- 숫자 두 개가 제대로 계산되는가

## 5. 책 연습문제 단계

CSV의 `11.6c, 7, 9, 10, 11`은 저장소 파일을 수정하는 단계가 아닙니다.

이 단계에서는 아래처럼 처리하면 됩니다.

- 책에서 문제를 읽고 손으로 풀이
- 네트워크/동시성/HTTP 흐름을 다시 점검
- 프록시 구현 전에 헷갈리는 개념을 정리

즉, 이 항목은 `proxy.c`로 넘어가기 전 개념 점검 단계로 생각하면 됩니다.

## 6. Proxy 서버 구현 순서

Tiny가 끝났다면 이제 [proxy.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/proxy.c)로 갑니다.

현재 `proxy.c`는 거의 비어 있습니다.

- 캐시 상수만 있음
- `user_agent_hdr`만 있음
- `main()`도 실제 서버 기능이 없음

따라서 프록시는 아래 순서로 직접 뼈대를 세우는 것이 가장 안전합니다.

### 6-1. 첫 번째 목표

가장 먼저 달성할 목표는 이것 하나입니다.

- 프록시를 통해 Tiny의 `home.html`을 정상적으로 가져온다

즉, 동시성과 캐시는 잠깐 잊고 "중계만 되는 프록시"부터 성공시켜야 합니다.

### 6-2. 추천 구현 순서

1. `main()`을 진짜 서버 루프로 바꾸기
2. 클라이언트 요청 하나를 처리하는 함수 만들기
3. URI에서 `host`, `port`, `path` 분리 helper 만들기
4. 원 서버로 전달할 새 요청 헤더 조립 helper 만들기
5. 원 서버 응답을 읽어 클라이언트로 그대로 전달하기
6. basic test 통과 후 스레드 추가
7. concurrency test 통과 후 캐시 추가

### 6-3. proxy.c에서 어디에 무엇을 추가하면 좋은가

가장 추천하는 구조는 아래와 같습니다.

- 상단: 필요한 `#include`
- 상단: helper 함수 선언
- 상단: URI 파싱용 helper
- 상단: 요청 헤더 조립 helper
- 중간: 클라이언트 1건 처리 함수
- 하단: `main()`

동시성과 캐시를 넣을 때는 아래를 추가합니다.

- 스레드 루틴 함수
- 캐시 구조체 정의
- 캐시 초기화 / 조회 / 삽입 함수

### 6-4. 순차 프록시 단계 힌트

#### `main()`

역할:
- 포트를 인자로 받아 리슨 소켓 생성
- 반복적으로 연결을 받고 처리 함수 호출

Tiny의 `main()`과 매우 비슷한 구조로 시작하면 됩니다.

완료 기준:
- 프록시 프로세스가 포트를 받아 켜지고, 클라이언트 연결을 받을 수 있다

#### 요청 처리 함수

역할:
- 브라우저 요청을 읽고 원 서버에 전달하고 응답을 다시 돌려줌

해야 할 일:
- 요청 라인 읽기
- 메소드 검사
- 절대 URI 파싱
- 원 서버 연결
- 새 요청 헤더 전송
- 원 서버 응답을 끝까지 읽고 클라이언트로 전달

힌트:
- 처음부터 모든 헤더를 완벽히 복제하려 하지 마세요.
- 필수 헤더만 안정적으로 조립하는 것이 먼저입니다.

막히는 포인트:
- 브라우저 요청 URI는 Tiny와 달리 보통 절대 URI입니다.
- `GET http://host:port/path HTTP/1.1`에서 path만 뽑아 원 서버에 보내야 합니다.

#### URI 파싱 helper

역할:
- 절대 URI를 `host`, `port`, `path`로 분해

힌트:
- `http://` 접두어 제거 후 생각하면 쉬워집니다.
- 포트가 생략된 경우 기본값을 무엇으로 둘지 미리 정하세요.

완료 기준:
- `http://localhost:8000/home.html`을 세 부분으로 나눌 수 있다

#### 요청 헤더 조립 helper

역할:
- 원 서버로 보낼 HTTP/1.0 요청 문자열 생성

반드시 신경 쓸 헤더:
- `Host`
- `User-Agent`
- `Connection: close`
- `Proxy-Connection: close`

힌트:
- 브라우저가 준 헤더를 무조건 다 복붙하지 말고, 충돌나는 헤더는 덮어쓰는 쪽이 안전합니다.
- 빈 줄 `\r\n`이 헤더 끝이라는 점을 항상 기억하세요.

#### 응답 전달 단계

역할:
- 원 서버 소켓에서 읽은 바이트를 그대로 브라우저 소켓에 전달

힌트:
- 이 단계는 텍스트가 아니라 "바이트 스트림 복사"라고 생각하면 됩니다.
- 먼저 캐시 없이, 읽은 즉시 쓰는 흐름으로 만드세요.

완료 기준:
- `curl --proxy http://localhost:<proxy-port> http://localhost:<tiny-port>/home.html`가 성공한다

## 7. Proxy 동시병렬 처리 단계

CSV의 다음 항목은 `구현 - proxy서버 - 동시병렬 처리`입니다.

이 단계는 `proxy.c` 안에서만 이어서 진행하면 됩니다.

### 추천 추가 순서

1. 스레드 루틴 함수 선언
2. `accept`한 `connfd`를 힙 메모리로 넘기기
3. 스레드 안에서 처리 함수 호출
4. 스레드 `detach`

핵심 힌트:
- `connfd` 주소를 지역변수 그대로 넘기면 경쟁 조건이 생기기 쉽습니다.
- "메인 스레드는 accept만" 하고, "작업 스레드는 요청 처리만" 한다고 역할을 나누면 깔끔합니다.

완료 기준:
- 느린 요청 하나가 있어도 다른 요청이 막히지 않는다
- `driver.sh`의 concurrency test가 통과한다

## 8. Proxy 캐시 단계

마지막 항목은 `구현 - proxy서버 - 캐싱 웹오브젝트`입니다.

이 단계도 역시 [proxy.c](/Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab/proxy.c)에서 진행합니다.

### 추천 구현 순서

1. 캐시 엔트리 구조체 설계
2. 전역 캐시 저장소 설계
3. 조회 함수 작성
4. 삽입 함수 작성
5. 교체 정책 추가
6. 동시성 보호 추가

### 캐시 구조에서 꼭 들어가야 할 것

- 어떤 URI의 응답인지 식별할 키
- 응답 데이터 본문
- 데이터 크기
- 최근 사용 시각 또는 우선순위 정보
- 비어 있는 슬롯인지 여부

### 캐시 삽입 시점 힌트

- 원 서버 응답을 읽으면서 동시에 누적
- 응답 전체를 다 받은 뒤
- 크기가 `MAX_OBJECT_SIZE` 이하일 때만 저장

이 순서가 중요한 이유:
- 덜 받은 응답을 캐시에 넣으면 다음 요청에서 깨진 데이터를 줄 수 있습니다.

### 동기화 힌트

- 캐시는 여러 스레드가 같이 쓰는 자원입니다.
- 처음에는 단순한 전역 락으로 시작해도 됩니다.
- 읽기/쓰기 락으로 세분화하는 것은 나중 단계로 미뤄도 됩니다.

완료 기준:
- Tiny를 종료한 뒤에도 캐시된 `home.html`은 프록시가 돌려준다
- `driver.sh`의 cache test가 통과한다

## 9. CSV 테스트 순서를 실제 명령으로 바꾸기

### Tiny 테스트

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab
cd tiny && make && cd ..
./tiny/tiny 8000
```

다른 터미널에서:

```bash
curl -v http://localhost:8000/home.html
curl -v "http://localhost:8000/cgi-bin/adder?123&456"
```

### Proxy 기본 테스트

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab
make
./tiny/tiny 8000
./proxy 4500
```

다른 터미널에서:

```bash
curl --proxy http://localhost:4500 http://localhost:8000/home.html
```

### 자동 채점

```bash
cd /Users/woonyong/workspace/Krafton-Jungle/SW_AI-W08-webproxy_lab/webproxy-lab
./driver.sh
```

## 10. 오늘 당장 뭘 하면 되나

지금 당장 가장 좋은 실행 순서는 아래입니다.

1. `tiny/tiny.c`에 `doit`, `read_requesthdrs`, `parse_uri`를 먼저 추가
2. `get_filetype`, `serve_static`, `clienterror`를 붙여 정적 페이지 성공
3. `serve_dynamic`를 붙여 CGI 성공
4. `adder.c` 동작 확인
5. 그 다음에야 `proxy.c`에서 순차 프록시 시작
6. basic test 통과 후 동시성
7. concurrency test 통과 후 캐시

즉, 지금 해야 할 첫 작업은 `proxy.c`가 아니라 `tiny/tiny.c`입니다.

## 11. 한 줄 체크포인트

단계별로 아래 질문에 "예"라고 답할 수 있으면 다음으로 넘어가면 됩니다.

- Tiny가 `home.html`을 줄 수 있는가
- Tiny가 `adder` CGI를 실행할 수 있는가
- Proxy가 Tiny 정적 파일을 중계할 수 있는가
- Proxy가 느린 요청 때문에 전체가 막히지 않는가
- Proxy가 Tiny 종료 후 캐시 응답을 돌려줄 수 있는가

이 다섯 개가 끝나면 CSV의 구현 핵심은 거의 완료된 것입니다.
