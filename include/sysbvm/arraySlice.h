#ifndef SYSBVM_ARRAY_SLICE_H
#define SYSBVM_ARRAY_SLICE_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_arraySlice_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t elements;
    sysbvm_tuple_t offset;
    sysbvm_tuple_t size;
} sysbvm_arraySlice_t;

/**
 * Creates an array slice
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_create(sysbvm_context_t *context, sysbvm_tuple_t elements, sysbvm_tuple_t offset, sysbvm_tuple_t count);

/**
 * Creates an array slice with the specified array.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_createWithOffsetAndSize(sysbvm_context_t *context, sysbvm_tuple_t elements, size_t offset, size_t count);

/**
 * Creates an array slice that points to an array of the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_createWithArrayOfSize(sysbvm_context_t *context, size_t size);

/**
 * Gets the size of an array slice.
 */
SYSBVM_API size_t sysbvm_arraySlice_getSize(sysbvm_tuple_t arraySlice);

/**
 * Gets an element from the array slice.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_at(sysbvm_tuple_t arraySlice, size_t index);

/**
 * Sets an element in the array slice.
 */
SYSBVM_API void sysbvm_arraySlice_atPut(sysbvm_tuple_t arraySlice, size_t index, sysbvm_tuple_t value);

/**
 * Makes an array slice from the specified position.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_at(sysbvm_tuple_t arraySlice, size_t index);

/**
 * Makes an array slice from the specified position.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_fromOffset(sysbvm_context_t *context, sysbvm_tuple_t arraySlice, size_t fromOffset);

/**
 * Makes an array with the content of this slice
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_asArray(sysbvm_context_t *context, sysbvm_tuple_t arraySlice);

#endif //SYSBVM_ARRAY_SLICE_H