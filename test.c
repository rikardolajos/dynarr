#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void* checked_malloc(size_t sz);
void* checked_realloc(void* p, size_t sz);
void assert_failed(const char* cond, const char* file, int line);

#define DYNARR_IMPLEMENTATION
#define DYNARR_MALLOC(sz) checked_malloc(sz)
#define DYNARR_REALLOC(p, sz) checked_realloc(p, sz)
#define DYNARR_ASSERT(cond)                                                    \
    ((cond) ? (void)0 : assert_failed(#cond, __FILE__, __LINE__))
#include "dynarr.h"

static int failures = 0;

/* Unlike assert(), this is not compiled away with NDEBUG */
#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            printf("  %s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);  \
            failures++;                                                        \
        }                                                                      \
    } while (0)

/* Check that stmt trips a DYNARR_ASSERT() */
static jmp_buf assert_jmp;
static int expect_assert = 0;

#define CHECK_ASSERTS(stmt)                                                    \
    do {                                                                       \
        expect_assert = 1;                                                     \
        if (setjmp(assert_jmp) == 0) {                                         \
            stmt;                                                              \
            printf("  %s:%d: expected assertion: %s\n", __FILE__, __LINE__,    \
                   #stmt);                                                     \
            failures++;                                                        \
        }                                                                      \
        expect_assert = 0;                                                     \
    } while (0)

void assert_failed(const char* cond, const char* file, int line)
{
    if (expect_assert) {
        longjmp(assert_jmp, 1);
    }
    printf("%s:%d: assertion failed: %s\n", file, line, cond);
    abort();
}

/* When fail_malloc or fail_realloc is set, the allocation returns NULL without
 * touching any memory, so allocation failures can be tested */
static int fail_malloc = 0;
static int malloc_calls = 0;

void* checked_malloc(size_t sz)
{
    malloc_calls++;
    if (fail_malloc)
        return NULL;
    void* p = malloc(sz);
    if (!p)
        printf("Unable to allocate memory\n");
    return p;
}

static int fail_realloc = 0;
static int realloc_calls = 0;
static size_t last_realloc_size = 0;

void* checked_realloc(void* p, size_t sz)
{
    realloc_calls++;
    last_realloc_size = sz;
    if (fail_realloc)
        return NULL;
    return realloc(p, sz);
}

void test_alloc()
{
    dynarr da = daalloc(int, 5);

    CHECK(da.data != NULL);
    CHECK(da.size != 0);
    CHECK(da.elemsize == sizeof(int));
    CHECK(da.count == 5);
    CHECK(da.capacity != 0);

    dafree(&da);
    CHECK(da.data == NULL);
    CHECK(da.size == 0);
    CHECK(da.elemsize == 0);
    CHECK(da.count == 0);
    CHECK(da.capacity == 0);
}

void test_alloc_in_if_else()
{
    /* Used to fail to compile, daalloc() expanded with a trailing ';' */
    int big = 1;
    dynarr da;
    if (big)
        da = daalloc(int, 8);
    else
        da = daalloc(int, 1);

    CHECK(da.count == 8);

    dafree(&da);
}

void test_alloc_failure()
{
    /* Used to assert, or crash in memset() with NDEBUG */
    fail_malloc = 1;
    dynarr da = daalloc(int, 5);
    fail_malloc = 0;

    CHECK(da.data == NULL);
    CHECK(da.size == 0);
    CHECK(da.elemsize == sizeof(int));
    CHECK(da.count == 0);
    CHECK(da.capacity == 0);

    /* The failed array can still be used */
    int i = 7;
    dapush(&da, &i);
    CHECK(da.count == 1);
    CHECK(daget(&da, 0, int) == 7);

    dafree(&da);
}

void test_alloc_size_overflow()
{
    /* elemsize * capacity used to wrap around to a tiny allocation */
    int calls = malloc_calls;
    dynarr da = _daalloc(SIZE_MAX / 2 + 1, 2);

    CHECK(malloc_calls == calls);
    CHECK(da.data == NULL);
    CHECK(da.count == 0);
    CHECK(da.capacity == 0);

    dafree(&da);
}

void test_reserve()
{
    dynarr da = daalloc(int, 0);

    dareserve(&da, 10);
    CHECK(da.capacity == 10);

    /* Reserving 0 must not call realloc(p, 0), which frees the data */
    uint8_t* data = da.data;
    int calls = realloc_calls;
    dareserve(&da, 0);
    CHECK(realloc_calls == calls);
    CHECK(da.data == data);
    CHECK(da.capacity == 10);

    /* Reserving less than or equal to the capacity does nothing */
    dareserve(&da, 5);
    dareserve(&da, 10);
    CHECK(realloc_calls == calls);
    CHECK(da.capacity == 10);

    dafree(&da);
}

void test_reserve_below_count()
{
    dynarr da = daalloc(int, 8);

    dareserve(&da, 2);
    CHECK(da.capacity == 8);
    CHECK(da.count == 8);

    dafree(&da);
}

void test_reserve_failure()
{
    dynarr da = daalloc(int, 0);
    uint8_t* data = da.data;

    fail_realloc = 1;
    dareserve(&da, 10);
    fail_realloc = 0;

    CHECK(da.data == data);
    CHECK(da.capacity == 1);

    dafree(&da);
}

void test_push()
{
    dynarr da = daalloc(int, 0);

    for (int i = 0; i < 10; i++) {
        dapush(&da, &i);
    }
    CHECK(da.count == 10);
    CHECK(da.capacity >= 10);
    CHECK(da.size == 10 * sizeof(int));

    for (int i = 0; i < 10; i++) {
        CHECK(daget(&da, i, int) == i);
    }

    dafree(&da);
}

void test_push_zero_capacity()
{
    /* Capacity 0 used to "grow" to 2 * 0 = 0 and then write out of bounds */
    dynarr da = {.elemsize = sizeof(int)};

    int i = 42;
    dapush(&da, &i);
    CHECK(da.count == 1);
    CHECK(da.capacity == 1);
    CHECK(daget(&da, 0, int) == 42);

    dafree(&da);
}

void test_push_growth_failure()
{
    dynarr da = daalloc(int, 0);

    int i = 1;
    dapush(&da, &i);
    CHECK(da.count == 1);
    CHECK(da.capacity == 1);

    /* The array is full and cannot grow, the push must be dropped */
    fail_realloc = 1;
    i = 2;
    dapush(&da, &i);
    fail_realloc = 0;

    CHECK(da.count == 1);
    CHECK(da.capacity == 1);
    CHECK(da.size == sizeof(int));
    CHECK(daget(&da, 0, int) == 1);

    dafree(&da);
}

void test_push_capacity_overflow()
{
    /* Fake full arrays at the capacity limits. No memory is ever touched since
     * realloc fails, and the pushes are dropped. */
    uint8_t b = 0;

    /* Doubling 2^31 used to wrap around to 0 */
    uint32_t half = UINT32_MAX / 2 + 1;
    dynarr da = {.elemsize = 1, .count = half, .capacity = half};

    fail_realloc = 1;
    dapush(&da, &b);
    fail_realloc = 0;

    CHECK(last_realloc_size == UINT32_MAX);
    CHECK(da.count == half);
    CHECK(da.capacity == half);

    /* Already at the maximum capacity, there is nothing to grow to */
    dynarr full = {.elemsize = 1, .count = UINT32_MAX, .capacity = UINT32_MAX};

    int calls = realloc_calls;
    dapush(&full, &b);

    CHECK(realloc_calls == calls);
    CHECK(full.count == UINT32_MAX);
}

void test_pop()
{
    dynarr da = daalloc(int, 0);

    for (int i = 0; i < 10; i++) {
        dapush(&da, &i);
    }

    int i = 10;
    while (da.count) {
        int j = dapop(&da, int);
        CHECK(j == --i);
    }
    CHECK(da.count == 0);
    CHECK(da.size == 0);
    CHECK(i == 0);

    /* Popping an empty array used to wrap count around to UINT32_MAX */
    CHECK_ASSERTS((void)dapop(&da, int));
    CHECK(da.count == 0);

    dafree(&da);
}

void test_get()
{
    dynarr da = daalloc(int, 0);

    int i = 8;
    dapush(&da, &i);

    int j = daget(&da, 0, int);
    CHECK(j == 8);

    /* Out-of-bounds, even though index 1 is within the capacity */
    dareserve(&da, 4);
    CHECK_ASSERTS((void)daget(&da, 1, int));

    dafree(&da);
}

void test_set()
{
    dynarr da = daalloc(int, 10);

    for (int i = 0; i < 10; i++) {
        daset(&da, i, int, 25);
    }

    while (da.count) {
        int k = dapop(&da, int);
        CHECK(k == 25);
    }

    /* Out-of-bounds */
    void* dst = daset(&da, 0, int, 1);
    CHECK(dst == NULL);

    dafree(&da);
}

typedef struct {
    int x;
    float y;
} point;

void test_set_struct()
{
    /* Used to fail to compile, the struct initialized its first member */
    dynarr da = daalloc(point, 2);

    point p = {3, 4.5f};
    void* dst = daset(&da, 1, point, p);
    CHECK(dst != NULL);

    point q = daget(&da, 1, point);
    CHECK(q.x == 3);
    CHECK(q.y == 4.5f);

    /* The other element is left zero-initialized */
    point r = daget(&da, 0, point);
    CHECK(r.x == 0);
    CHECK(r.y == 0.0f);

    dafree(&da);
}

int main()
{
    printf("Testing daalloc() and dafree()\n");
    test_alloc();
    test_alloc_in_if_else();
    test_alloc_failure();
    test_alloc_size_overflow();

    printf("Testing dareserve()\n");
    test_reserve();
    test_reserve_below_count();
    test_reserve_failure();

    printf("Testing dapush()\n");
    test_push();
    test_push_zero_capacity();
    test_push_growth_failure();
    test_push_capacity_overflow();

    printf("Testing dapop()\n");
    test_pop();

    printf("Testing daget()\n");
    test_get();

    printf("Testing daset()\n");
    test_set();
    test_set_struct();

    if (failures) {
        printf("=== DYNARR TESTS FAILED: %d check(s) ===\n", failures);
        return 1;
    }

    printf("=== DYNARR TESTS COMPLETED ===\n");

    return 0;
}
