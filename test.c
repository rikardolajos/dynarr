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
    dynarr dv = daalloc(sizeof(int), 5);

    assert(dv.data != NULL);
    assert(dv.size == 0);
    assert(dv.elemsize == sizeof(int));
    assert(dv.count == 0);
    assert(dv.capacity == 5);

    dafree(&dv);
    assert(dv.data == NULL);
    assert(dv.size == 0);
    assert(dv.elemsize == 0);
    assert(dv.count == 0);
    assert(dv.capacity == 0);
}

void test_reserve()
{
    dynarr dv = daalloc(sizeof(int), 1);

    dareserve(&dv, 10);
    assert(dv.capacity == 10);

    dafree(&dv);
}

void test_push()
{
    dynarr dv = daalloc(sizeof(int), 1);

    for (int i = 0; i < 10; i++) {
        dapush(&dv, &i);
    }
    assert(dv.count == 10);

    dafree(&dv);
}

void test_pop()
{
    dynarr dv = daalloc(sizeof(int), 1);

    for (int i = 0; i < 10; i++) {
        int j = 91;
        dapush(&dv, &j);
    }

    int i = 0;
    while (dv.count) {
        dapop(&dv, NULL);
        i++;
    }
    assert(dv.count == 0);
    assert(i == 10);

    dafree(&dv);
}

void test_get()
{
    dynarr dv = daalloc(sizeof(int), 1);

    int i = 8;
    dapush(&dv, &i);

    int j;
    daget(&dv, 0, &j);
    assert(j == 8);

    dafree(&dv);
}

void test_set()
{
    dynarr dv = daalloc(sizeof(int), 10);

    for (int i = 0; i < 10; i++) {
        int j = 25;
        daset(&dv, i, &j);
    }

    while (dv.count) {
        int k;
        dapop(&dv, &k);
        assert(k == 25);
    }

    dafree(&dv);
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
