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
 * using dvfree().
 *
 *  elemsize    is the size of each element in bytes
 *  count       is the reserved capacity of the array
 */
dynarr daalloc(size_t elemsize, uint32_t count);

/* Free allocated memory of the dynamic array and reset fields to zero.
 *
 *  dv      is the dynamic array to free
 */
void dvfree(dynarr* dv);

/* Reserve the capacity of a dynamic array to a new given capacity. On failure
 * nothing happens.
 *
 *  dv      is the dynamic array to reserve
 *  newcap  is the new capacity in number of elements
 */
void dvreserve(dynarr* dv, uint32_t newcap);

/* Push an element to the back of the dynamic array.
 *
 *  dv      is the dynamic array to push to
 *  elem    is the new element to push
 */
void dvpush(dynarr* dv, const void* elem);

/* Pop an element from the back of the dynamic array.
 *
 *  dv      is the dynamic array to pop from
 *  elem    is where the element is popped to
 */
void dvpop(dynarr* dv, void* elem);

/* Get an element at index i from the dynamic array.
 *
 *  dv      is the dynamic array get from
 *  i       is the index
 *  elem    is where the element is returned to
 */
void dvget(dynarr* dv, uint32_t i, void* elem);

/* Set an element at index i in the dynamic array.
 *
 *  dv      is the dynamic array set to
 *  i       is the index
 *  elem    is the new element set
 */
void dvset(dynarr* dv, uint32_t i, const void* elem);

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

void dafree(dynarr* dv)
{
    if (!dv) {
        return;
    }

    DYNARR_FREE(dv->data);
    memset(dv, 0, sizeof(*dv));
}

void dareserve(dynarr* dv, uint32_t newcap)
{
    if (!dv) {
        return;
    }

    dv->capacity = newcap;
    size_t newsize = dv->elemsize * dv->capacity;
    uint8_t* temp = DYNARR_REALLOC(dv->data, newsize);
    if (temp) {
        dv->data = temp;
    }
}

void dapush(dynarr* dv, void* elem)
{
    if (!dv || !elem) {
        return;
    }

    if (dv->count == dv->capacity) {
        dareserve(dv, 2 * dv->capacity);
    }

    memcpy(dv->data + dv->count * dv->elemsize, elem, dv->elemsize);
    dv->count += 1;
    dv->size = dv->elemsize * dv->count;
}

void dapop(dynarr* dv, void* elem)
{
    if (!dv) {
        return;
    }

    dv->count -= 1;
    dv->size = dv->elemsize * dv->count;

    if (elem)
        memcpy(elem, dv->data + dv->count * dv->elemsize, dv->elemsize);
}

void daget(dynarr* dv, uint32_t i, void* elem)
{
    if (!dv || !elem) {
        return;
    }

    memcpy(elem, dv->data + i * dv->elemsize, dv->elemsize);
}

void daset(dynarr* dv, uint32_t i, const void* elem)
{
    if (!dv || !elem) {
        return;
    }

    if (i >= dv->count) {
        dv->count = i + 1;
        dv->size = dv->elemsize * dv->count;
    }

    memcpy(dv->data + i * dv->elemsize, elem, dv->elemsize);
}

#endif /* DYNARR_IMPLEMENTATION */
