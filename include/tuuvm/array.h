#ifndef TUUVM_ARRAY_H
#define TUUVM_ARRAY_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_array_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t elements[];
} tuuvm_array_t;

typedef struct tuuvm_byteArray_s
{
    tuuvm_tuple_header_t header;
    uint8_t elements[];
} tuuvm_byteArray_t;

/**
 * Creates an array with the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_array_create(tuuvm_context_t *context, tuuvm_tuple_t slotCount);

/**
 * Gets an element from an array.
 */
TUUVM_API tuuvm_tuple_t tuuvm_array_at(tuuvm_tuple_t array, size_t index);

/**
 * Creates a byte array with the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_byteArray_create(tuuvm_context_t *context, tuuvm_tuple_t size);

/**
 * Gets an element from a byte array.
 */
TUUVM_API uint8_t tuuvm_byteArray_at(tuuvm_tuple_t array, size_t index);

/**
 * Gets an element from an array or byte array.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arrayOrByteArray_at(tuuvm_tuple_t array, size_t index);

/**
 * Sets an element in an array or byte array.
 */
TUUVM_API void tuuvm_arrayOrByteArray_atPut(tuuvm_tuple_t array, size_t index, tuuvm_tuple_t value);

#endif //TUUVM_ARRAY_H