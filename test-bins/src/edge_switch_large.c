#include <stdio.h>

const char *day_name(int d) {
    switch (d) {
    case 0: return "Sunday";
    case 1: return "Monday";
    case 2: return "Tuesday";
    case 3: return "Wednesday";
    case 4: return "Thursday";
    case 5: return "Friday";
    case 6: return "Saturday";
    default: return "invalid";
    }
}

int big_switch(int x) {
    switch (x) {
    case 0:  return x + 100;
    case 1:  return x * 2;
    case 2:  return x ^ 0xFF;
    case 3:  return x & 0x0F;
    case 4:  return x | 0xF0;
    case 5:  return x - 10;
    case 6:  return x + 20;
    case 7:  return x * 3;
    case 8:  return x ^ 0xAA;
    case 9:  return x & 0x55;
    case 10: return x | 0x33;
    case 11: return x - 5;
    case 12: return x + 12;
    case 13: return x * 7;
    case 14: return x ^ 0xCC;
    case 15: return x & 0x77;
    case 16: return x | 0x88;
    case 17: return x - 17;
    case 18: return x + 18;
    case 19: return x * 11;
    case 20: return x ^ 0xDD;
    case 21: return x & 0xEE;
    case 22: return x | 0x11;
    case 23: return x - 23;
    case 24: return x + 24;
    default: return -1;
    }
}

int main(void) {
    for (int i = 0; i < 8; i++)
        printf("day(%d) = %s\n", i, day_name(i));
    for (int i = 0; i < 26; i++)
        printf("big(%d) = %d\n", i, big_switch(i));
    return 0;
}
