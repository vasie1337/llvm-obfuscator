#include <stdio.h>
#include <stdint.h>
#include <limits.h>

int safe_add(int a, int b) { return a + b; }
int safe_sub(int a, int b) { return a - b; }
int safe_xor(int a, int b) { return a ^ b; }
int safe_and(int a, int b) { return a & b; }
int safe_or(int a, int b)  { return a | b; }

uint32_t unsigned_ops(uint32_t a, uint32_t b) {
    uint32_t r = a + b;
    r ^= (a - b);
    r &= (a | b);
    r |= (a & b);
    return r;
}

int main(void) {
    printf("add(INT_MAX, 1) = %d\n", safe_add(INT_MAX, 1));
    printf("add(INT_MIN, -1) = %d\n", safe_add(INT_MIN, -1));
    printf("sub(INT_MIN, 1) = %d\n", safe_sub(INT_MIN, 1));
    printf("sub(0, INT_MIN) = %d\n", safe_sub(0, INT_MIN));
    printf("xor(-1, -1) = %d\n", safe_xor(-1, -1));
    printf("xor(INT_MAX, INT_MIN) = %d\n", safe_xor(INT_MAX, INT_MIN));
    printf("and(-1, 0) = %d\n", safe_and(-1, 0));
    printf("or(0, 0) = %d\n", safe_or(0, 0));

    printf("add(0, 0) = %d\n", safe_add(0, 0));
    printf("sub(0, 0) = %d\n", safe_sub(0, 0));
    printf("xor(0, 0) = %d\n", safe_xor(0, 0));
    printf("and(0, 0) = %d\n", safe_and(0, 0));
    printf("or(0, 0) = %d\n", safe_or(0, 0));

    printf("add(-1, -1) = %d\n", safe_add(-1, -1));
    printf("sub(-1, -1) = %d\n", safe_sub(-1, -1));

    printf("unsigned(UINT32_MAX, 1) = %u\n", unsigned_ops(UINT32_MAX, 1));
    printf("unsigned(0, 0) = %u\n", unsigned_ops(0, 0));
    printf("unsigned(0x55555555, 0xAAAAAAAA) = %u\n",
           unsigned_ops(0x55555555, 0xAAAAAAAA));

    return 0;
}
