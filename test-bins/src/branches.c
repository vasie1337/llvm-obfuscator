#include <stdio.h>

const char *classify(int x) {
    switch (x % 5) {
    case 0: return "fizz";
    case 1: return "buzz";
    case 2: return "fizzbuzz";
    case 3: return "none";
    default: return "other";
    }
}

int collatz_steps(int n) {
    int steps = 0;
    while (n != 1) {
        if (n % 2 == 0)
            n /= 2;
        else
            n = 3 * n + 1;
        steps++;
    }
    return steps;
}

int main(void) {
    for (int i = 0; i < 10; i++)
        printf("classify(%d) = %s\n", i, classify(i));

    printf("collatz(27) = %d steps\n", collatz_steps(27));
    printf("collatz(1000) = %d steps\n", collatz_steps(1000));
    return 0;
}
