#ifndef TUUVM_ENVIRONMENT_H
#define TUUVM_ENVIRONMENT_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_environment_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t parent;
    tuuvm_tuple_t symbolTable;
} tuuvm_environment_t;

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_create(tuuvm_context_t *context, tuuvm_tuple_t parent);

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_getIntrinsicsBuiltInEnvironment(tuuvm_context_t *context);

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForEvaluation(tuuvm_context_t *context);

/**
 * Looks a symbol recursively on an environment.
 */ 
TUUVM_API void tuuvm_environment_setSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t binding);

/**
 * Looks a symbol recursively on an environment.
 */ 
TUUVM_API bool tuuvm_environment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding);

#endif //TUUVM_ENVIRONMENT_H
