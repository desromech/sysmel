#ifndef TUUVM_ARRAY_SLICE_H
#define TUUVM_ARRAY_SLICE_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_arraySlice_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t elements;
    tuuvm_tuple_t offset;
    tuuvm_tuple_t size;
} tuuvm_arraySlice_t;

/**
 * Creates an array slice
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_create(tuuvm_context_t *context, tuuvm_tuple_t elements, tuuvm_tuple_t offset, tuuvm_tuple_t count);

/**
 * Creates an array slice with the specified array.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_createWithOffsetAndSize(tuuvm_context_t *context, tuuvm_tuple_t elements, size_t offset, size_t count);

/**
 * Gets the size of an array slice.
 */
TUUVM_API size_t tuuvm_arraySlice_getSize(tuuvm_tuple_t arraySlice);

/**
 * Gets an element from the array slice.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_at(tuuvm_tuple_t arraySlice, size_t index);

#endif //TUUVM_ARRAY_SLICE_H