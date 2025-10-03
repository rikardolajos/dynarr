void* checked_malloc(size_t sz);

#define DYNARR_IMPLEMENTATION
#define DYNARR_MALLOC(sz) checked_malloc(sz)
#include "dynarr.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void* checked_malloc(size_t sz)
{
    void* p = malloc(sz);
    if (!p)
        printf("Unable to allocate memory\n");
    return p;
}

void test_alloc()
{
    dynarr da = daalloc(sizeof(int), 5);

    assert(da.data != NULL);
    assert(da.size != 0);
    assert(da.elemsize == sizeof(int));
    assert(da.count == 5);
    assert(da.capacity != 0);

    dafree(&da);
    assert(da.data == NULL);
    assert(da.size == 0);
    assert(da.elemsize == 0);
    assert(da.count == 0);
    assert(da.capacity == 0);
}

void test_reserve()
{
    dynarr da = daalloc(sizeof(int), 0);

    dareserve(&da, 10);
    assert(da.capacity == 10);

    dafree(&da);
}

void test_push()
{
    dynarr da = daalloc(sizeof(int), 0);

    for (int i = 0; i < 10; i++) {
        dapush(&da, &i);
    }
    assert(da.count == 10);

    dafree(&da);
}

void test_pop()
{
    dynarr da = daalloc(sizeof(int), 0);

    for (int i = 0; i < 10; i++) {
        int j = 91;
        dapush(&da, &j);
    }

    int i = 0;
    while (da.count) {
        dapop(&da, int);
        i++;
    }
    assert(da.count == 0);
    assert(i == 10);

    dafree(&da);
}

void test_get()
{
    dynarr da = daalloc(sizeof(int), 0);

    int i = 8;
    dapush(&da, &i);

    int j = daget(&da, 0, int);
    assert(j == 8);

    dafree(&da);
}

void test_set()
{
    dynarr da = daalloc(sizeof(int), 10);

    for (int i = 0; i < 10; i++) {
        daset(&da, i, int, 25);
    }

    while (da.count) {
        int k = dapop(&da, int);
        assert(k == 25);
    }

    /* Out-of-bounds */
    void* dst = daset(&da, 0, int, 1);

    if (dst) {
        printf("Error: expected NULL on out-of-bounds\n");
    }

    dafree(&da);
}

int main()
{
    printf("Testing daalloc() and dafree()\n");
    test_alloc();

    printf("Testing dareserve()\n");
    test_reserve();

    printf("Testing dapush()\n");
    test_push();

    printf("Testing dapop()\n");
    test_pop();

    printf("Testing daget()\n");
    test_get();

    printf("Testing daset()\n");
    test_set();

    printf("=== DYNARR TESTS COMPLETED ===\n");

    return 0;
}
