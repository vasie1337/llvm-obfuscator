#include <stdio.h>

int ternary_chain(int a, int b, int c) {
    int x = (a > b) ? a : b;
    int y = (x > c) ? x : c;
    int z = (a < 0) ? -a : a;
    int w = (b < 0) ? -b : b;
    return (y > 50) ? (z + w) : (z - w);
}

int cond_accumulate(int n) {
    int acc = 0;
    for (int i = 0; i < n; i++) {
        int v = (i % 2 == 0) ? i * 3 : i * 7;
        int w = (i % 3 == 0) ? v + 1 : v - 1;
        int x = (i % 5 == 0) ? w ^ 0xFF : w & 0xFF;
        acc += (x > 100) ? x : -x;
    }
    return acc;
}

int multi_exit(int x) {
    if (x == 0) return 0;
    if (x == 1) return 1;
    if (x < 0)  return -x;
    if (x > 100) return x / 2;
    if (x % 2)  return x * 3 + 1;
    return x / 2;
}

int main(void) {
    printf("ternary(10, 20, 15) = %d\n", ternary_chain(10, 20, 15));
    printf("ternary(-5, 3, 100) = %d\n", ternary_chain(-5, 3, 100));
    printf("ternary(0, 0, 0) = %d\n", ternary_chain(0, 0, 0));

    printf("cond_acc(50) = %d\n", cond_accumulate(50));
    printf("cond_acc(100) = %d\n", cond_accumulate(100));

    int tests[] = {0, 1, -5, 50, 150, 7, 12};
    for (int i = 0; i < 7; i++)
        printf("multi_exit(%d) = %d\n", tests[i], multi_exit(tests[i]));

    return 0;
}
