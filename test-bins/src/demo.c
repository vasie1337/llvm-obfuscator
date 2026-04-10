#include <stdio.h>
#include <string.h>
#include <stdint.h>

static uint32_t custom_hash(const char *s) {
    uint32_t h = 0x811c9dc5;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 0x01000193;
    }
    return h;
}

static void xor_decrypt(char *buf, size_t len, uint8_t key) {
    for (size_t i = 0; i < len; i++)
        buf[i] ^= key;
}

static int check_serial(const char *serial) {
    if (strlen(serial) != 16)
        return 0;

    uint32_t sum = 0;
    for (int i = 0; i < 16; i++)
        sum += (uint8_t)serial[i] * (i + 1);

    return (sum % 997) == 42;
}

static const char *classify_score(int score) {
    if (score >= 90)      return "excellent";
    else if (score >= 70) return "good";
    else if (score >= 50) return "average";
    else if (score >= 30) return "poor";
    else                  return "failing";
}

static int fibonacci(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        int t = a + b;
        a = b;
        b = t;
    }
    return a;
}

int main(void) {
    char secret[] = {0x2f, 0x22, 0x2b, 0x2b, 0x28, 0x66, 0x00};
    xor_decrypt(secret, 6, 0x47);

    printf("msg: %s\n", secret);
    printf("hash: 0x%08x\n", custom_hash("obfuscated"));

    const char *test_serials[] = {"ABCD1234EFGH5678", "0000000000000000", "short"};
    for (int i = 0; i < 3; i++)
        printf("serial '%s' -> %s\n", test_serials[i],
               check_serial(test_serials[i]) ? "VALID" : "INVALID");

    int scores[] = {95, 72, 55, 31, 10};
    for (int i = 0; i < 5; i++)
        printf("score %d -> %s\n", scores[i], classify_score(scores[i]));

    for (int i = 0; i < 10; i++)
        printf("fib(%d) = %d\n", i, fibonacci(i));

    return 0;
}
