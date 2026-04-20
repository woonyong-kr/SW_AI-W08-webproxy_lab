#include <stdio.h>

/* 권장 캐시 크기와 객체 최대 크기 */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* 이 긴 줄을 코드에 그대로 넣어도 스타일 점수는 깎이지 않습니다 */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main()
{
  printf("%s", user_agent_hdr);
  return 0;
}
