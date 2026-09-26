/* Copyright 2024 Rikard Olajos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef DYNARR_MALLOC
#define DYNARR_MALLOC(sz) malloc(sz)
#endif

#ifndef DYNARR_REALLOC
#define DYNARR_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef DYNARR_FREE
#define DYNARR_FREE(p) free(p)
#endif

#ifndef DYNARR_ASSERT
#define DYNARR_ASSERT(cond) assert(cond)
#endif

/* A dynamic array. The fields should not be written to by the user.
 *
 * dapush() and dareserve() may move the data, which invalidates any pointer
 * into it, including pointers returned by dapush() and daset().
 */
typedef struct {
    uint8_t* data;   /* Data stored */
    size_t elemsize; /* Element size in bytes */
    size_t count;    /* Number of elements */
    size_t capacity; /* Capacity in number of elements */
} dynarr;

/* The macros below that take a type check that sizeof(type) matches the
 * element size of the dynamic array. This catches most, but not all, type
 * mismatches, since different types can have the same size (e.g. int and
 * float).
 */

/* Allocate a new dynamic array. An allocated dynamic array has to be freed
 * using dafree(). Elements are zero-initialized.
 *
 * On failure data is NULL and count and capacity are 0. The array can still be
 * pushed to (which tries to allocate again) and freed.
 *
 *  type    is the type of the elements
 *  count   is the number of elements to start with
 */
#define daalloc(type, count) (daalloc_(sizeof(type), (count)))
dynarr daalloc_(size_t elemsize, size_t count);

/* Free allocated memory of the dynamic array and reset fields to zero.
 *
 *  da      is the dynamic array to free
 */
void dafree(dynarr* da);

/* Size in bytes of the elements in the dynamic array (elemsize * count),
 * excluding any extra reserved capacity.
 *
 *  da      is the dynamic array
 */
#define dasize(da) ((da)->elemsize * (da)->count)

/* Reserve the capacity of a dynamic array to a new given capacity. The array
 * never shrinks, so nothing happens if capacity is not larger than the current
 * capacity. On failure nothing happens.
 *
 *  da          is the dynamic array to reserve
 *  capacity    is the new capacity in number of elements
 */
void dareserve(dynarr* da, size_t capacity);

/* Push an element to the back of the dynamic array. Returns a pointer to where
 * the element is written, or NULL if the array is full and cannot grow, in
 * which case the array is left unchanged.
 *
 *  da      is the dynamic array to push to
 *  type    is the type of the elements
 *  elem    is the new element to push
 */
#define dapush(da, type, elem) (dapush_((da), (type[]){(elem)}, sizeof(type)))
void* dapush_(dynarr* da, const void* elem, size_t elemsize);

/* Pop an element from the back of the dynamic array.
 *
 *  da      is the dynamic array to pop from, must not be empty
 *  type    is the type of the elements
 */
#define dapop(da, type) (*((type*)dapop_((da), sizeof(type))))
void* dapop_(dynarr* da, size_t elemsize);

/* Get an element at index i from the dynamic array.
 *
 *  da      is the dynamic array to get from
 *  type    is the type of the elements
 *  i       is the index, has to be less than da->count
 */
#define daget(da, type, i) (*((type*)daget_((da), sizeof(type), (i))))
void* daget_(dynarr* da, size_t elemsize, size_t i);

/* Set an element at index i in the dynamic array. Returns a pointer to where
 * the element is written, or NULL on failure.
 *
 *  da      is the dynamic array to set to
 *  type    is the type of the elements
 *  i       is the index, has to be less than da->count
 *  elem    is the new element set
 */
#define daset(da, type, i, elem)                                               \
    (daset_((da), sizeof(type), (i), (type[]){(elem)}))
void* daset_(dynarr* da, size_t elemsize, size_t i, const void* elem);

#ifdef DYNARR_IMPLEMENTATION

dynarr daalloc_(size_t elemsize, size_t count)
{
    size_t capacity = count;

    if (capacity < 1) {
        capacity = 1;
    }

    /* Return an empty array on failure, keeping elemsize so it can be pushed */
    dynarr empty = {.elemsize = elemsize};

    if (elemsize == 0 || capacity > SIZE_MAX / elemsize) {
        return empty;
    }

    void* data = DYNARR_MALLOC(elemsize * capacity);
    if (!data) {
        return empty;
    }

    memset(data, 0, elemsize * capacity);

    return (dynarr){
        .data = data,
        .elemsize = elemsize,
        .count = count,
        .capacity = capacity,
    };
}

void dafree(dynarr* da)
{
    DYNARR_ASSERT(da);

    DYNARR_FREE(da->data);
    memset(da, 0, sizeof(*da));
}

void dareserve(dynarr* da, size_t capacity)
{
    DYNARR_ASSERT(da);

    /* Never shrink (this also avoids realloc() with size 0, which frees the
     * data), and refuse sizes that do not fit in size_t */
    if (capacity <= da->capacity || da->elemsize == 0 ||
        capacity > SIZE_MAX / da->elemsize) {
        return;
    }

    size_t newsize = da->elemsize * capacity;
    uint8_t* temp = DYNARR_REALLOC(da->data, newsize);
    if (temp) {
        da->capacity = capacity;
        da->data = temp;
    }
}

void* dapush_(dynarr* da, const void* elem, size_t elemsize)
{
    DYNARR_ASSERT(da);
    DYNARR_ASSERT(elem);
    DYNARR_ASSERT(elemsize == da->elemsize);

    if (da->count == da->capacity) {
        /* Double the capacity, without wrapping around on overflow */
        size_t capacity = SIZE_MAX;
        if (da->capacity == 0) {
            capacity = 1;
        } else if (da->capacity <= SIZE_MAX / 2) {
            capacity = 2 * da->capacity;
        }
        dareserve(da, capacity);

        /* Could not grow, drop the element rather than write out of bounds */
        if (da->count == da->capacity) {
            return NULL;
        }
    }

    void* dst = da->data + da->count * da->elemsize;
    da->count += 1;

    return memcpy(dst, elem, da->elemsize);
}

void* dapop_(dynarr* da, size_t elemsize)
{
    DYNARR_ASSERT(da);
    DYNARR_ASSERT(elemsize == da->elemsize);
    DYNARR_ASSERT(da->count > 0);

    da->count -= 1;

    return da->data + da->count * da->elemsize;
}

void* daget_(dynarr* da, size_t elemsize, size_t i)
{
    DYNARR_ASSERT(da);
    DYNARR_ASSERT(elemsize == da->elemsize);
    DYNARR_ASSERT(i < da->count);

    return da->data + i * da->elemsize;
}

void* daset_(dynarr* da, size_t elemsize, size_t i, const void* elem)
{
    DYNARR_ASSERT(da);
    DYNARR_ASSERT(elem);
    DYNARR_ASSERT(elemsize == da->elemsize);

    if (i >= da->count) {
        return NULL;
    }

    return memcpy(da->data + i * da->elemsize, elem, da->elemsize);
}

#endif /* DYNARR_IMPLEMENTATION */
