#include <stdio.h>

typedef int (*binop_t)(int, int);

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }
static int xor(int a, int b) { return a ^ b; }

static binop_t ops[] = {add, sub, mul, xor};
static const char *names[] = {"add", "sub", "mul", "xor"};

int apply_chain(int x, binop_t *fns, int n) {
    for (int i = 0; i < n; i++)
        x = fns[i](x, i + 1);
    return x;
}

int main(void) {
    for (int i = 0; i < 4; i++)
        printf("%s(10, 3) = %d\n", names[i], ops[i](10, 3));

    printf("chain(100) = %d\n", apply_chain(100, ops, 4));

    binop_t selected = (7 > 3) ? add : sub;
    printf("selected(5, 2) = %d\n", selected(5, 2));

    return 0;
}
