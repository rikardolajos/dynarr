void *checked_malloc(size_t sz);

#define DYNVEC_IMPLEMENTATION
#define DYNVEC_MALLOC(sz) checked_malloc(sz)
#include "dynvec.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void *checked_malloc(size_t sz)
{
    void *p = malloc(sz);
    if (!p)
        printf("Unable to allocate memory\n");
    return p;
}

void test_alloc()
{
    dynvec dv = dvalloc(sizeof(int), 5);

    assert(dv.data != NULL);
    assert(dv.size == 0);
    assert(dv.elemsize == sizeof(int));
    assert(dv.count == 0);
    assert(dv.capacity == 5);

    dvfree(&dv);
    assert(dv.data == NULL);
    assert(dv.size == 0);
    assert(dv.elemsize == 0);
    assert(dv.count == 0);
    assert(dv.capacity == 0);
}

void test_reserve()
{
    dynvec dv = dvalloc(sizeof(int), 1);

    dvreserve(&dv, 10);
    assert(dv.capacity == 10);

    dvfree(&dv);
}

void test_push()
{
    dynvec dv = dvalloc(sizeof(int), 1);

    for (int i = 0; i < 10; i++)
    {
        dvpush(&dv, &i);
    }
    assert(dv.count == 10);

    dvfree(&dv);
}

void test_pop()
{
    dynvec dv = dvalloc(sizeof(int), 1);

    for (int i = 0; i < 10; i++)
    {
        int j = 91;
        dvpush(&dv, &j);
    }

    int i = 0;
    while (dv.count)
    {
        dvpop(&dv, NULL);
        i++;
    }
    assert(dv.count == 0);
    assert(i == 10);

    dvfree(&dv);
}

void test_get()
{
    dynvec dv = dvalloc(sizeof(int), 1);

    int i = 8;
    dvpush(&dv, &i);

    int j;
    dvget(&dv, 0, &j);
    assert(j == 8);

    dvfree(&dv);
}

void test_set()
{
    dynvec dv = dvalloc(sizeof(int), 10);

    for (int i = 0; i < 10; i++)
    {
        int j = 25;
        dvset(&dv, i, &j);
    }

    while (dv.count)
    {
        int k;
        dvpop(&dv, &k);
        assert(k == 25);
    }

    dvfree(&dv);
}

int main()
{
    printf("Testing dvalloc() and dvfree()\n");
    test_alloc();

    printf("Testing dvreserve()\n");
    test_reserve();

    printf("Testing dvpush()\n");
    test_push();

    printf("Testing dvpop()\n");
    test_pop();

    printf("Testing dvget()\n");
    test_get();

    printf("Testing dvset()\n");
    test_set();

    printf("=== DYNVEC TESTS COMPLETED ===\n");

    return 0;
}