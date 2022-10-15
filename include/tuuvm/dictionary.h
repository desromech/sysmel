#ifndef TUUVM_DICTIONARY_H
#define TUUVM_DICTIONARY_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_dictionary_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
    tuuvm_tuple_t equalsFunction;
    tuuvm_tuple_t hashFunction;
} tuuvm_dictionary_t;

typedef size_t (*tuuvm_dictionary_explicitHashFunction_t)(void *element);
typedef bool (*tuuvm_dictionary_explicitEqualsFunction_t)(void *element, tuuvm_tuple_t dictionaryElement);

/**
 * Creates a hash dictionary data structure that uses the specified equals and hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_dictionary_create(tuuvm_context_t *context, tuuvm_tuple_t equalsFunction, tuuvm_tuple_t hashFunction);

/**
 * Creates a hash dictionary data structure that uses the identity equals and identity hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_identityDictionary_create(tuuvm_context_t *context);

/**
 * Inserts an element in the dictionary.
 */ 
TUUVM_API bool tuuvm_dictionary_find(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outValue);

/**
 * Inserts an element in the dictionary.
 */ 
TUUVM_API void tuuvm_dictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value);

#endif //TUUVM_DICTIONARY_H