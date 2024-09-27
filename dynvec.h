#pragma once

#include <memory.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef DYNVEC_MALLOC
#define DYNVEC_MALLOC(sz) malloc(sz)
#endif

#ifndef DYNVEC_REALLOC
#define DYNVEC_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef DYNVEC_FREE
#define DYNVEC_FREE(p) free(p)
#endif

typedef struct
{
    uint8_t *data;     /* Data stored */
    size_t size;       /* Size in bytes of used array (elemsize * count) */
    size_t elemsize;   /* Element size in bytes */
    uint32_t count;    /* Number of elements */
    uint32_t capacity; /* Capacity in number of elements */
} dynvec;

/* Allocate a new dynamic vector. An allocated dynamic vector has to be freed
 * using dvfree().
 *
 *  elemsize    is the size of each element in bytes
 *  count       is the reserved capacity of the vector
 */
dynvec dvalloc(size_t elemsize, uint32_t count);

/* Free allocated memory of the dynamic vector and reset fields to zero.
 *
 *  dv      is the dynamic vector to free
 */
void dvfree(dynvec *dv);

/* Reserve the capacity of a dynamic vector to a new given capacity. On failure
 * nothing happens.
 *
 *  dv      is the dynamic vector to reserve
 *  newcap  is the new capacity in number of elements
 */
void dvreserve(dynvec *dv, uint32_t newcap);

/* Push an element to the back of the dynamic vector.
 *
 *  dv      is the dynamic vector to push to
 *  elem    is the new element to push
 */
void dvpush(dynvec *dv, const void *elem);

/* Pop an element from the back of the dynamic vector.
 *
 *  dv      is the dynamic vector to pop from
 *  elem    is where the element is popped to
 */
void dvpop(dynvec *dv, void *elem);

/* Get an element at index i from the dynamic vector.
 *
 *  dv      is the dynamic vector get from
 *  i       is the index
 *  elem    is where the element is returned to
 */
void dvget(dynvec *dv, uint32_t i, void *elem);

/* Set an element at index i in the dynamic vector.
 *
 *  dv      is the dynamic vector set to
 *  i       is the index
 *  elem    is the new element set
 */
void dvset(dynvec *dv, uint32_t i, const void *elem);

#ifdef DYNVEC_IMPLEMENTATION

dynvec dvalloc(size_t elemsize, uint32_t count)
{
    if (count < 1)
        count = 1;
    return (dynvec){
        .data = DYNVEC_MALLOC(elemsize * count),
        .elemsize = elemsize,
        .capacity = count,
    };
}

void dvfree(dynvec *dv)
{
    if (!dv)
        return;

    DYNVEC_FREE(dv->data);
    memset(dv, 0, sizeof(*dv));
}

void dvreserve(dynvec *dv, uint32_t newcap)
{
    if (!dv)
        return;

    dv->capacity = newcap;
    size_t newsize = dv->elemsize * dv->capacity;
    uint8_t *temp = DYNVEC_REALLOC(dv->data, newsize);
    if (temp)
        dv->data = temp;
}

void dvpush(dynvec *dv, void *elem)
{
    if (!dv || !elem)
        return;

    if (dv->count == dv->capacity)
        dvreserve(dv, 2 * dv->capacity);

    memcpy(dv->data + dv->count * dv->elemsize, elem, dv->elemsize);
    dv->count += 1;
    dv->size = dv->elemsize * dv->count;
}

void dvpop(dynvec *dv, void *elem)
{
    if (!dv)
        return;

    dv->count -= 1;
    dv->size = dv->elemsize * dv->count;

    if (elem)
        memcpy(elem, dv->data + dv->count * dv->elemsize, dv->elemsize);
}

void dvget(dynvec *dv, uint32_t i, void *elem)
{
    if (!dv || !elem)
        return;

    memcpy(elem, dv->data + i * dv->elemsize, dv->elemsize);
}

void dvset(dynvec *dv, uint32_t i, const void *elem)
{
    if (!dv || !elem)
        return;

    if (i >= dv->count)
    {
        dv->count = i + 1;
        dv->size = dv->elemsize * dv->count;
    }

    memcpy(dv->data + i * dv->elemsize, elem, dv->elemsize);
}

#endif /* DYNVEC_IMPLEMENTATION */
