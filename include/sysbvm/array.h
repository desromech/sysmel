#ifndef SYSBVM_ARRAY_H
#define SYSBVM_ARRAY_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_array_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t elements[];
} sysbvm_array_t;

typedef struct sysbvm_byteArray_s
{
    sysbvm_tuple_header_t header;
    uint8_t elements[];
} sysbvm_byteArray_t;

/**
 * Creates an array with the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_array_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount);

/**
 * Creates a weak array with the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_weakArray_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount);

/**
 * Gets an element from an array.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_array_at(sysbvm_tuple_t array, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_array_t*)array)->elements[index];
}
/**
 * Sets an element in an array.
 */
SYSBVM_INLINE void sysbvm_array_atPut(sysbvm_tuple_t array, size_t index, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return;
    
    ((sysbvm_array_t*)array)->elements[index] = value;
}

/**
 * Gets the array size.
 */
SYSBVM_INLINE size_t sysbvm_array_getSize(sysbvm_tuple_t array)
{
    return sysbvm_tuple_getSizeInSlots(array);
}

/**
 * Converts the array into an array slice.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_array_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t array);

/**
 * Gets an array with the first elements
 */
SYSBVM_API sysbvm_tuple_t sysbvm_array_getFirstElements(sysbvm_context_t *context, sysbvm_tuple_t array, size_t size);

/**
 * Creates a byte array with the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_byteArray_create(sysbvm_context_t *context, sysbvm_tuple_t size);

/**
 * Gets an element from a byte array.
 */
SYSBVM_API uint8_t sysbvm_byteArray_at(sysbvm_tuple_t array, size_t index);

/**
 * Gets an element from an array or byte array.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_arrayOrByteArray_at(sysbvm_tuple_t array, size_t index);

/**
 * Sets an element in an array or byte array.
 */
SYSBVM_API void sysbvm_arrayOrByteArray_atPut(sysbvm_tuple_t array, size_t index, sysbvm_tuple_t value);

/**
 * Makes an array slice ignore first N elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_array_fromOffset(sysbvm_context_t *context, sysbvm_tuple_t arraySlice, size_t fromOffset);

#endif //SYSBVM_ARRAY_H