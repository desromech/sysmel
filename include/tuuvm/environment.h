#ifndef TUUVM_ENVIRONMENT_H
#define TUUVM_ENVIRONMENT_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_environment_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t parent;
    tuuvm_tuple_t symbolTable;
} tuuvm_environment_t;

typedef struct tuuvm_symbolBinding_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t name;
} tuuvm_symbolBinding_t;

typedef struct tuuvm_symbolArgumentBinding_s
{
    tuuvm_symbolBinding_t super;
} tuuvm_symbolArgumentBinding_t;

typedef struct tuuvm_symbolValueBinding_s
{
    tuuvm_symbolBinding_t super;
    tuuvm_tuple_t value;
} tuuvm_symbolValueBinding_t;

/**
 * Is the symbol binding a value?
 */
TUUVM_API bool tuuvm_symbolBinding_isValue(tuuvm_context_t *context, tuuvm_tuple_t binding);

/**
 * Creates a symbol argument binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolArgumentBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name);

/**
 * Creates a symbol value binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value);

/**
 * Gets the value from the symbol value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolValueBinding_getValue(tuuvm_tuple_t binding)
{
    return ((tuuvm_symbolValueBinding_t*)binding)->value;
}

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
 * Sets a new symbol binding in the environment.
 */ 
TUUVM_API void tuuvm_environment_setNewSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t binding);

/**
 * Sets a symbol binding in the environment.
 */ 
TUUVM_API void tuuvm_environment_setSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t binding);

/**
 * Sets a new symbol binding with value the environment.
 */ 
TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value);

/**
 * Sets a symbol binding with value in the environment.
 */ 
TUUVM_API void tuuvm_environment_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value);

/**
 * Looks a symbol recursively on an environment.
 */ 
TUUVM_API bool tuuvm_environment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding);

#endif //TUUVM_ENVIRONMENT_H
