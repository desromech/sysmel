#ifndef SYSBVM_SET_H
#define SYSBVM_SET_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_set_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t size;
    sysbvm_tuple_t storage;
} sysbvm_set_t;

typedef sysbvm_set_t sysbvm_identitySet_t;

typedef size_t (*sysbvm_identitySet_explicitHashFunction_t)(sysbvm_context_t *context, void *element);
typedef bool (*sysbvm_identitySet_explicitEqualsFunction_t)(void *element, sysbvm_tuple_t setElement);

/**
 * Creates a hash set data structure that uses the specified equals and hash function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_identitySet_create(sysbvm_context_t *context);

/**
 * Inserts an element in the set.
 */ 
SYSBVM_API bool sysbvm_identitySet_findWithExplicitHash(sysbvm_context_t *context, sysbvm_tuple_t set, void *element, sysbvm_identitySet_explicitHashFunction_t hashFunction, sysbvm_identitySet_explicitEqualsFunction_t equalsFunction, sysbvm_tuple_t *outFoundElement);

/**
 * Inserts an element in the set.
 */ 
SYSBVM_API bool sysbvm_identitySet_find(sysbvm_tuple_t set, sysbvm_tuple_t element, sysbvm_tuple_t *outFoundElement);

/**
 * Inserts an element in the set.
 */ 
SYSBVM_API void sysbvm_identitySet_insert(sysbvm_context_t *context, sysbvm_tuple_t set, sysbvm_tuple_t element);

#endif //SYSBVM_SET_H