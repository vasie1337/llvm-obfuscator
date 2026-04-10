#include <stdio.h>
#include <string.h>

int main(void) {
    printf("%s\n", "A");
    printf("%s\n", "AB");
    printf("%s\n", "The quick brown fox jumps over the lazy dog. "
                    "This is a longer string to test encryption of "
                    "multi-line constant data that spans many bytes.");
    printf("%s\n", "Special chars: \t\n\\\"\'");
    printf("%s\n", "\x01\x02\x03\x7f");

    const char *strs[] = {
        "alpha", "bravo", "charlie", "delta", "echo",
        "foxtrot", "golf", "hotel", "india", "juliet"
    };
    int total = 0;
    for (int i = 0; i < 10; i++) {
        printf("str[%d] = %s (len=%zu)\n", i, strs[i], strlen(strs[i]));
        total += (int)strlen(strs[i]);
    }
    printf("total length: %d\n", total);

    return 0;
}
