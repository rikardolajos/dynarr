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

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef DYNARR_MALLOC
#define DYNARR_MALLOC(sz) malloc(sz)
#endif

#ifndef DYNARR_REALLOC
#define DYNARR_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef DYNARR_FREE
#define DYNARR_FREE(p) free(p)
#endif

typedef struct {
    uint8_t* data;     /* Data stored */
    size_t size;       /* Size in bytes of used array (elemsize * count) */
    size_t elemsize;   /* Element size in bytes */
    uint32_t count;    /* Number of elements */
    uint32_t capacity; /* Capacity in number of elements */
} dynarr;

/* Allocate a new dynamic array. An allocated dynamic array has to be freed
 * using dafree().
 *
 *  elemsize    is the size of each element in bytes
 *  count       is the reserved capacity of the array
 */
dynarr daalloc(size_t elemsize, uint32_t count);

/* Free allocated memory of the dynamic array and reset fields to zero.
 *
 *  da      is the dynamic array to free
 */
void dafree(dynarr* da);

/* Reserve the capacity of a dynamic array to a new given capacity. On failure
 * nothing happens.
 *
 *  da      is the dynamic array to reserve
 *  newcap  is the new capacity in number of elements
 */
void dareserve(dynarr* da, uint32_t newcap);

/* Push an element to the back of the dynamic array.
 *
 *  da      is the dynamic array to push to
 *  elem    is the new element to push
 */
void dapush(dynarr* da, const void* elem);

/* Pop an element from the back of the dynamic array.
 *
 *  da      is the dynamic array to pop from
 */
#define dapop(da, type) (*((type*)_dapop((da))))
void* _dapop(dynarr* da);

/* Get an element at index i from the dynamic array.
 *
 *  da      is the dynamic array to get from
 *  i       is the index
 */
#define daget(da, i, type) (*((type*)_daget((da), (i))))
void* _daget(dynarr* da, uint32_t i);

/* Set an element at index i in the dynamic array.
 *
 *  da      is the dynamic array to set to
 *  i       is the index
 *  elem    is the new element set
 */
void daset(dynarr* da, uint32_t i, const void* elem);

#ifdef DYNARR_IMPLEMENTATION

dynarr daalloc(size_t elemsize, uint32_t count)
{
    if (count < 1) {
        count = 1;
    }

    return (dynarr){
        .data = DYNARR_MALLOC(elemsize * count),
        .elemsize = elemsize,
        .capacity = count,
    };
}

void dafree(dynarr* da)
{
    if (!da) {
        return;
    }

    DYNARR_FREE(da->data);
    memset(da, 0, sizeof(*da));
}

void dareserve(dynarr* da, uint32_t newcap)
{
    if (!da) {
        return;
    }

    da->capacity = newcap;
    size_t newsize = da->elemsize * da->capacity;
    uint8_t* temp = DYNARR_REALLOC(da->data, newsize);
    if (temp) {
        da->data = temp;
    }
}

void dapush(dynarr* da, void* elem)
{
    if (!da || !elem) {
        return;
    }

    if (da->count == da->capacity) {
        dareserve(da, 2 * da->capacity);
    }

    memcpy(da->data + da->count * da->elemsize, elem, da->elemsize);
    da->count += 1;
    da->size = da->elemsize * da->count;
}

void* _dapop(dynarr* da)
{
    if (!da) {
        return NULL;
    }

    da->count -= 1;
    da->size = da->elemsize * da->count;

    return da->data + da->count * da->elemsize;
}

void* _daget(dynarr* da, uint32_t i)
{
    if (!da) {
        return NULL;
    }

    return da->data + i * da->elemsize;
}

void daset(dynarr* da, uint32_t i, const void* elem)
{
    if (!da || !elem) {
        return;
    }

    if (i >= da->count) {
        da->count = i + 1;
        da->size = da->elemsize * da->count;
    }

    memcpy(da->data + i * da->elemsize, elem, da->elemsize);
}

#endif /* DYNARR_IMPLEMENTATION */
