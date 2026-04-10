#include <stdio.h>
#include <stdlib.h>

void fatal(const char *msg) __attribute__((noreturn));

void fatal(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int checked_div(int a, int b) {
    if (b == 0)
        fatal("division by zero");
    return a / b;
}

int process(int x) {
    if (x < 0)
        fatal("negative input");
    if (x == 0)
        return 0;
    if (x > 1000)
        fatal("input too large");
    return checked_div(x * x + x, x);
}

int main(void) {
    printf("process(1) = %d\n", process(1));
    printf("process(10) = %d\n", process(10));
    printf("process(100) = %d\n", process(100));
    printf("process(999) = %d\n", process(999));
    printf("process(0) = %d\n", process(0));
    return 0;
}
