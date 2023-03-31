#ifndef TUUVM_DICTIONARY_H
#define TUUVM_DICTIONARY_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_dictionary_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_dictionary_t;

typedef tuuvm_dictionary_t tuuvm_identityDictionary_t;
typedef tuuvm_dictionary_t tuuvm_methodDictionary_t;
typedef tuuvm_dictionary_t tuuvm_weakValueDictionary_t;

typedef size_t (*tuuvm_dictionary_explicitHashFunction_t)(void *element);
typedef bool (*tuuvm_dictionary_explicitEqualsFunction_t)(void *element, tuuvm_tuple_t dictionaryElement);

/**
 * Creates a dictionary.
 */
TUUVM_API tuuvm_tuple_t tuuvm_dictionary_create(tuuvm_context_t *context);

/**
 * Creates a dictionary with storage for the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_dictionary_createWithCapacity(tuuvm_context_t *context, size_t expectedCapacity);

/**
 * Finds an element in the dictionary.
 */
TUUVM_API bool tuuvm_dictionary_find(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outValue);

/**
 * Adds an association to the dictionary
 */
TUUVM_API void tuuvm_dictionary_add(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t association);

/**
 * Inserts an element in the dictionary.
 */ 
TUUVM_API void tuuvm_dictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Creates a weak value dictionary.
 */
TUUVM_API tuuvm_tuple_t tuuvm_weakValueDictionary_create(tuuvm_context_t *context);

/**
 * Finds an element in the weak value dictionary.
 */
TUUVM_API bool tuuvm_weakValueDictionary_find(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outValue);

/**
 * Inserts an element in the dictionary.
 */ 
TUUVM_API void tuuvm_weakValueDictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Creates a hash dictionary data structure that uses the identity equals and identity hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_identityDictionary_create(tuuvm_context_t *context);

/**
 * Creates a method dictionary data structure.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_methodDictionary_create(tuuvm_context_t *context);

/**
 * Creates a method dictionary data structure with the specified capacity.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_methodDictionary_createWithCapacity(tuuvm_context_t *context, size_t initialCapacity);

/**
 * Finds an association in the dictionary.
 */ 
TUUVM_API bool tuuvm_identityDictionary_findAssociation(tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outAssociation);

/**
 * Adds an association to the dictionary.
 */ 
TUUVM_API void tuuvm_identityDictionary_addAssociation(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t association);

/**
 * Finds an element in the dictionary.
 */ 
TUUVM_API bool tuuvm_identityDictionary_find(tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outValue);

/**
 * Inserts an element in the dictionary.
 */ 
TUUVM_API void tuuvm_identityDictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Finds an element in the method dictionary.
 */ 
TUUVM_API bool tuuvm_methodDictionary_find(tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outValue);

/**
 * Inserts an element in the method dictionary.
 */ 
TUUVM_API void tuuvm_methodDictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value);

#endif //TUUVM_DICTIONARY_H