#ifndef TUUVM_ARRAY_LIST_H
#define TUUVM_ARRAY_LIST_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_arrayList_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_arrayList_t;

/**
 * Creates an array list
 */
TUUVM_API tuuvm_tuple_t tuuvm_arrayList_create(tuuvm_context_t *context);

/**
 * Adds an element to the array slice.
 */
TUUVM_API void tuuvm_arrayList_add(tuuvm_context_t *context, tuuvm_tuple_t arrayList, tuuvm_tuple_t element);

/**
 * Converts the array listo into an array slice.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arrayList_asArraySlice(tuuvm_context_t *context, tuuvm_tuple_t arrayList);

/**
 * Gets the size of an array list.
 */
TUUVM_API size_t tuuvm_arrayList_getSize(tuuvm_tuple_t arrayList);

/**
 * Gets the capacity an array list.
 */
TUUVM_API size_t tuuvm_arrayList_getCapacity(tuuvm_tuple_t arrayList);

/**
 * Gets the array list element at the specified index.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arrayList_at(tuuvm_tuple_t arrayList, size_t index);

#endif //TUUVM_ARRAY_LIST_H