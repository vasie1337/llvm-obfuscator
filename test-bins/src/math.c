#include <stdio.h>

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int xor_(int a, int b) { return a ^ b; }
int and_(int a, int b) { return a & b; }
int or_(int a, int b) { return a | b; }

int compute(int x) {
    int result = 0;
    for (int i = 0; i < x; i++) {
        result = add(result, i);
        result = xor_(result, i * 3);
        result = or_(result, i & 0xFF);
        result = and_(result, 0x7FFFFFFF);
    }
    return result;
}

int main(void) {
    printf("add(10, 20) = %d\n", add(10, 20));
    printf("sub(50, 17) = %d\n", sub(50, 17));
    printf("xor(0xAA, 0x55) = 0x%X\n", xor_(0xAA, 0x55));
    printf("and(0xFF, 0x0F) = 0x%X\n", and_(0xFF, 0x0F));
    printf("or(0xF0, 0x0F) = 0x%X\n", or_(0xF0, 0x0F));
    printf("compute(100) = %d\n", compute(100));
    return 0;
}
