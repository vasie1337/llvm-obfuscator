#include <stdio.h>

static int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static int popcount(unsigned int x) {
    int c = 0;
    while (x) {
        c += x & 1;
        x >>= 1;
    }
    return c;
}

static unsigned int bitreverse(unsigned int x) {
    unsigned int r = 0;
    for (int i = 0; i < 32; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

int main(void) {
    int r = 0;
    r += gcd(48, 18);
    r += gcd(100, 75);
    r += gcd(17, 13);
    r += popcount(0xDEADBEEF);
    r += popcount(0);
    r += popcount(0xFFFFFFFF);
    r ^= (int)bitreverse(0x01);
    r ^= (int)bitreverse(0x80000000);
    return r & 0x7F;
}
