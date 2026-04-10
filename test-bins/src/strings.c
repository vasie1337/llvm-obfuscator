#include <stdio.h>
#include <string.h>

void reverse(char *s, int len) {
    for (int i = 0; i < len / 2; i++) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}

int main(void) {
    char buf[] = "Hello, World!";
    printf("original: %s\n", buf);
    reverse(buf, strlen(buf));
    printf("reversed: %s\n", buf);

    int sum = 0;
    for (int i = 0; buf[i]; i++)
        sum += buf[i];
    printf("char sum: %d\n", sum);

    return 0;
}
