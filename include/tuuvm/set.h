#ifndef TUUVM_SET_H
#define TUUVM_SET_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_set_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_set_t;

typedef tuuvm_set_t tuuvm_identitySet_t;

typedef size_t (*tuuvm_identitySet_explicitHashFunction_t)(void *element);
typedef bool (*tuuvm_identitySet_explicitEqualsFunction_t)(void *element, tuuvm_tuple_t setElement);

/**
 * Creates a hash set data structure that uses the specified equals and hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_identitySet_create(tuuvm_context_t *context);

/**
 * Inserts an element in the set.
 */ 
TUUVM_API bool tuuvm_identitySet_findWithExplicitHash(tuuvm_tuple_t set, void *element, tuuvm_identitySet_explicitHashFunction_t hashFunction, tuuvm_identitySet_explicitEqualsFunction_t equalsFunction, tuuvm_tuple_t *outFoundElement);

/**
 * Inserts an element in the set.
 */ 
TUUVM_API bool tuuvm_identitySet_find(tuuvm_tuple_t set, tuuvm_tuple_t element, tuuvm_tuple_t *outFoundElement);

/**
 * Inserts an element in the set.
 */ 
TUUVM_API void tuuvm_identitySet_insert(tuuvm_context_t *context, tuuvm_tuple_t set, tuuvm_tuple_t element);

#endif //TUUVM_SET_H