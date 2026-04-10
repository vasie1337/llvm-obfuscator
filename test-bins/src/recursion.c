#include <stdio.h>

long fib(int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

long factorial(int n) {
    if (n <= 1)
        return 1;
    return n * factorial(n - 1);
}

int main(void) {
    for (int i = 0; i < 15; i++)
        printf("fib(%d) = %ld\n", i, fib(i));

    for (int i = 1; i <= 12; i++)
        printf("%d! = %ld\n", i, factorial(i));

    return 0;
}
