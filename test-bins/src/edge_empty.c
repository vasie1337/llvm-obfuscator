#include <stdio.h>

void do_nothing(void) {
    return;
}

int return_constant(void) {
    return 42;
}

int main(void) {
    do_nothing();
    printf("constant: %d\n", return_constant());
    return 0;
}
