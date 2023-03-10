#ifndef TUUVM_ASSOCIATION_H
#define TUUVM_ASSOCIATION_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_association_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t key;
    tuuvm_tuple_t value;
} tuuvm_association_t;

/**
 * Creates an association.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_association_create(tuuvm_context_t *context, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Gets the key from the association.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_association_getKey(tuuvm_tuple_t association)
{
    if(!tuuvm_tuple_isNonNullPointer(association) || tuuvm_tuple_getSizeInSlots(association) < 1) return TUUVM_NULL_TUPLE;
    return ((tuuvm_association_t*)association)->key;
}

/**
 * Gets the value from the association.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_association_getValue(tuuvm_tuple_t association)
{
    if(!tuuvm_tuple_isNonNullPointer(association) || tuuvm_tuple_getSizeInSlots(association) < 2) return TUUVM_NULL_TUPLE;
    return ((tuuvm_association_t*)association)->value;
}

/**
 * Sets the value in an association.
 */
TUUVM_INLINE void tuuvm_association_setValue(tuuvm_tuple_t association, tuuvm_tuple_t newValue)
{
    if(!tuuvm_tuple_isNonNullPointer(association) || tuuvm_tuple_getSizeInSlots(association) < 2)
        return;

    ((tuuvm_association_t*)association)->value = newValue;
}

#endif //TUUVM_ASSOCIATION_H