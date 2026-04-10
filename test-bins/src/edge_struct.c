#include <stdio.h>
#include <string.h>

typedef struct {
    int x, y;
} Point;

typedef struct {
    char name[32];
    int values[8];
    int count;
} Record;

Point add_points(Point a, Point b) {
    return (Point){a.x + b.x, a.y + b.y};
}

int dot_product(Point a, Point b) {
    return a.x * b.x + a.y * b.y;
}

int record_sum(const Record *r) {
    int s = 0;
    for (int i = 0; i < r->count; i++)
        s += r->values[i];
    return s;
}

int array_reduce(int *arr, int n) {
    int acc = 0;
    for (int i = 0; i < n; i++) {
        acc ^= arr[i];
        acc += arr[i] & 0xFF;
    }
    return acc;
}

int main(void) {
    Point a = {3, 4}, b = {10, 20};
    Point c = add_points(a, b);
    printf("add_points: (%d, %d)\n", c.x, c.y);
    printf("dot_product: %d\n", dot_product(a, b));

    Record r;
    strcpy(r.name, "test_record");
    r.count = 5;
    for (int i = 0; i < r.count; i++)
        r.values[i] = (i + 1) * 10;
    printf("record '%s' sum: %d\n", r.name, record_sum(&r));

    int arr[] = {0xFF, 0xAA, 0x55, 0x01, 0x80, 0x7F, 0x00, 0xFE};
    printf("array_reduce: %d\n", array_reduce(arr, 8));

    return 0;
}
