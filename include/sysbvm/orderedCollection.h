#ifndef SYSBVM_ARRAY_LIST_H
#define SYSBVM_ARRAY_LIST_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_orderedCollection_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t size;
    sysbvm_tuple_t storage;
} sysbvm_orderedCollection_t;

/**
 * Creates an array list
 */
SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_create(sysbvm_context_t *context);

/**
 * Adds an element to the array slice.
 */
SYSBVM_API void sysbvm_orderedCollection_add(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection, sysbvm_tuple_t element);

/**
 * Converts the array list into an array slice.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection);

/**
 * Converts the array list into an array.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_asArray(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection);

/**
 * Converts the array list into a word array.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_asWordArray(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection);

/**
 * Gets the size of an array list.
 */
SYSBVM_API size_t sysbvm_orderedCollection_getSize(sysbvm_tuple_t orderedCollection);

/**
 * Gets the capacity an array list.
 */
SYSBVM_API size_t sysbvm_orderedCollection_getCapacity(sysbvm_tuple_t orderedCollection);

/**
 * Gets the array list element at the specified index.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_at(sysbvm_tuple_t orderedCollection, size_t index);

/**
 * Sets the array list element at the specified index.
 */
SYSBVM_API void sysbvm_orderedCollection_atPut(sysbvm_tuple_t orderedCollection, size_t index, sysbvm_tuple_t value);

/**
 * Does it include the specified element
 */
SYSBVM_API bool sysbvm_orderedCollection_identityIncludes(sysbvm_tuple_t orderedCollection, sysbvm_tuple_t element);

#endif //SYSBVM_ARRAY_LIST_H