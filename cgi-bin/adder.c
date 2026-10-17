#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* 1. 환경 변수 QUERY_STRING에서 쿼리 문자열 추출 */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&'); /* 앰퍼샌드('&')를 기준으로 두 인자를 분리 */
        if (p) {
            *p = '\0';
            strcpy(arg1, buf);
            strcpy(arg2, p + 1);
            n1 = atoi(arg1);  /* 문자열을 정수로 변환 */
            n2 = atoi(arg2);
        }
    }

    /* 2. 응답 바디(HTML 내용) 조립 */
    sprintf(content, "Welcome to add.com: THE Internet addition portal.\r\n<p>"
                     "The answer is: %d + %d = %d\r\n<p>"
                     "Thanks for visiting!\r\n",
            n1, n2, n1 + n2);

    /* 3. HTTP 응답 헤더와 바디를 표준 출력(stdout)으로 내보냄 */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout); /* 버퍼에 남아있는 데이터를 즉시 플러시 */

    exit(0);
}