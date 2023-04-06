#ifndef SYSBVM_ASSOCIATION_H
#define SYSBVM_ASSOCIATION_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_association_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t key;
    sysbvm_tuple_t value;
} sysbvm_association_t;

typedef sysbvm_association_t sysbvm_weakValueAssociation_t;

/**
 * Creates an association.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_association_create(sysbvm_context_t *context, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Create a weak value association.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_weakValueAssociation_create(sysbvm_context_t *context, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Gets the key from the association.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_association_getKey(sysbvm_tuple_t association)
{
    if(!sysbvm_tuple_isNonNullPointer(association) || sysbvm_tuple_getSizeInSlots(association) < 1) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_association_t*)association)->key;
}

/**
 * Gets the value from the association.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_association_getValue(sysbvm_tuple_t association)
{
    if(!sysbvm_tuple_isNonNullPointer(association) || sysbvm_tuple_getSizeInSlots(association) < 2) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_association_t*)association)->value;
}

/**
 * Sets the value in an association.
 */
SYSBVM_INLINE void sysbvm_association_setValue(sysbvm_tuple_t association, sysbvm_tuple_t newValue)
{
    if(!sysbvm_tuple_isNonNullPointer(association) || sysbvm_tuple_getSizeInSlots(association) < 2)
        return;

    ((sysbvm_association_t*)association)->value = newValue;
}

#endif //SYSBVM_ASSOCIATION_H