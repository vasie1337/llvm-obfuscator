#include <stdio.h>

int deeply_nested(int x) {
    int r = 0;
    if (x > 0) {
        if (x > 10) {
            if (x > 100) {
                for (int i = 0; i < x; i++) {
                    if (i % 2 == 0) {
                        if (i % 3 == 0) {
                            r += i * 7;
                        } else {
                            r += i * 3;
                        }
                    } else {
                        r -= i;
                    }
                }
            } else {
                for (int i = 0; i < x; i++) {
                    if (i & 1)
                        r ^= i;
                    else
                        r |= i;
                }
            }
        } else {
            r = x * x;
        }
    } else {
        if (x < -10) {
            r = -x;
            for (int i = 0; i > x; i--) {
                r += i & 0xFF;
            }
        } else {
            r = x + 1;
        }
    }
    return r;
}

int main(void) {
    int tests[] = {0, 1, 5, 15, 50, 150, -1, -5, -20};
    for (int i = 0; i < 9; i++)
        printf("nested(%d) = %d\n", tests[i], deeply_nested(tests[i]));
    return 0;
}
