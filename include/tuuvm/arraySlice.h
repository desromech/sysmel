#ifndef TUUVM_ARRAY_SLICE_H
#define TUUVM_ARRAY_SLICE_H

#pragma once

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
 * Creates an array slice that points to an array of the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_createWithArrayOfSize(tuuvm_context_t *context, size_t size);

/**
 * Gets the size of an array slice.
 */
TUUVM_API size_t tuuvm_arraySlice_getSize(tuuvm_tuple_t arraySlice);

/**
 * Gets an element from the array slice.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_at(tuuvm_tuple_t arraySlice, size_t index);

/**
 * Sets an element in the array slice.
 */
TUUVM_API void tuuvm_arraySlice_atPut(tuuvm_tuple_t arraySlice, size_t index, tuuvm_tuple_t value);

/**
 * Makes an array slice from the specified position.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_at(tuuvm_tuple_t arraySlice, size_t index);

/**
 * Makes an array slice from the specified position.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_fromOffset(tuuvm_context_t *context, tuuvm_tuple_t arraySlice, size_t fromOffset);

/**
 * Makes an array with the content of this slice
 */
TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_asArray(tuuvm_context_t *context, tuuvm_tuple_t arraySlice);

#endif //TUUVM_ARRAY_SLICE_H