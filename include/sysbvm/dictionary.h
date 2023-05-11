#ifndef SYSBVM_DICTIONARY_H
#define SYSBVM_DICTIONARY_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_dictionary_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t size;
    sysbvm_tuple_t storage;
} sysbvm_dictionary_t;

typedef sysbvm_dictionary_t sysbvm_identityDictionary_t;
typedef sysbvm_dictionary_t sysbvm_methodDictionary_t;
typedef sysbvm_dictionary_t sysbvm_weakValueDictionary_t;

typedef size_t (*sysbvm_dictionary_explicitHashFunction_t)(void *element);
typedef bool (*sysbvm_dictionary_explicitEqualsFunction_t)(void *element, sysbvm_tuple_t dictionaryElement);

/**
 * Creates a dictionary.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_dictionary_create(sysbvm_context_t *context);

/**
 * Creates a dictionary with storage for the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_dictionary_createWithCapacity(sysbvm_context_t *context, size_t expectedCapacity);

/**
 * Finds an element in the dictionary.
 */
SYSBVM_API bool sysbvm_dictionary_find(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue);

/**
 * Adds an association to the dictionary
 */
SYSBVM_API void sysbvm_dictionary_add(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t association);

/**
 * Inserts an element in the dictionary.
 */ 
SYSBVM_API void sysbvm_dictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Creates a weak value dictionary.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_weakValueDictionary_create(sysbvm_context_t *context);

/**
 * Finds an element in the weak value dictionary.
 */
SYSBVM_API bool sysbvm_weakValueDictionary_find(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue);

/**
 * Finds an association in the dictionary.
 */ 
SYSBVM_API bool sysbvm_weakValueDictionary_findAssociation(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outAssociation);

/**
 * Inserts an element in the dictionary.
 */ 
SYSBVM_API void sysbvm_weakValueDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Creates a hash dictionary data structure that uses the identity equals and identity hash function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_identityDictionary_create(sysbvm_context_t *context);

/**
 * Creates a method dictionary data structure.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_methodDictionary_create(sysbvm_context_t *context);

/**
 * Creates a method dictionary data structure with the specified capacity.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_methodDictionary_createWithCapacity(sysbvm_context_t *context, size_t initialCapacity);

/**
 * Finds an association in the dictionary.
 */ 
SYSBVM_API bool sysbvm_identityDictionary_findAssociation(sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outAssociation);

/**
 * Adds an association to the dictionary.
 */ 
SYSBVM_API void sysbvm_identityDictionary_addAssociation(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t association);

/**
 * Finds an element in the dictionary.
 */ 
SYSBVM_API bool sysbvm_identityDictionary_find(sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue);

/**
 * Inserts an element in the dictionary.
 */ 
SYSBVM_API void sysbvm_identityDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Finds an element in the method dictionary.
 */ 
SYSBVM_API bool sysbvm_methodDictionary_find(sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue);

/**
 * Inserts an element in the method dictionary.
 */ 
SYSBVM_API void sysbvm_methodDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value);

#endif //SYSBVM_DICTIONARY_H