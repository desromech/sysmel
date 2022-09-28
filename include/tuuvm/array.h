#ifndef TUUVM_ARRAY_H
#define TUUVM_ARRAY_H

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
 * Creates a byte array with the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_byteArray_create(tuuvm_context_t *context, tuuvm_tuple_t size);

#endif //TUUVM_ARRAY_H