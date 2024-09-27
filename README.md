# dynvec

This is a small header-only library that implements a dynamic vector in C. 
It can store generic types and has some basic functionality for pushing, popping, getting, and setting elements in the vector.


## Usage

Copy the header file `dynvec.h` to your repository and include it in the files where you want to use it.
In **exactly one** C file you will have to define `DYNVEC_IMPLEMENTATION` before including the header file:

```C
#define DYNVEC_IMPLEMENTATION
#include "dynvec.h"
```

Allocate a dynamic vector with `dvalloc()`, and free the memory resources with `dvfree()` after use.
Access the fields of the `dynvec` struct to see the details of the vector:

```C
typedef struct
{
    uint8_t *data;     /* Data stored */
    size_t size;       /* Size in bytes of used array (elemsize * count) */
    size_t elemsize;   /* Element size in bytes */
    uint32_t count;    /* Number of elements */
    uint32_t capacity; /* Capacity in number of elements */
} dynvec;
```

Notice that the `size` field contains the size in bytes of the used portion of the array (i.e., excluding any extra reserved capacity).
This differs from the C++ `std::vector` where `size` means the number of elements.
For `dynvec`, the number of elements can be read from `count`.
These fields should not be written to by the user.

For details of the implementation and which functions are available, check the header file.

Some examples of usage can be found in the `test.c` file.


## Safety

No safety is guaranteed in `dynvec` and no error messages are returned.
`dynvec` should be thought of as a small quality-of-life improvement over just using `malloc()` and `free()` directly.
It may fail silently, for instance `dvreserve()` will not give an error message if it failed to increase the capacity.


## License

`dynvec` is released under the MIT License.
