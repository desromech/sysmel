#ifndef TUUVM_FUNCTION_H
#define TUUVM_FUNCTION_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef tuuvm_tuple_t (*tuuvm_functionEntryPoint_t)(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments);

typedef enum tuuvm_functionFlags_e
{
    TUUVM_FUNCTION_FLAGS_NONE = 0,
    TUUVM_FUNCTION_FLAGS_MACRO = 1<<0,
    TUUVM_FUNCTION_FLAGS_VARIADIC = 1<<1,
} tuuvm_functionFlags_t;

typedef struct tuuvm_primitiveFunction_s
{
    tuuvm_tuple_header_t header;
    size_t argumentCount;
    size_t flags;
    void *userdata;
    tuuvm_functionEntryPoint_t entryPoint;
} tuuvm_primitiveFunction_t;

typedef struct tuuvm_closureASTFunction_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t flags;
    tuuvm_tuple_t closureEnvironment;
    tuuvm_tuple_t argumentSymbols;
    tuuvm_tuple_t body;
} tuuvm_closureASTFunction_t;

#define TUUVM_MAX_FUNCTION_ARGUMENTS 16

/**
 * Creates a primitive function tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

/**
 * Creates a function that uses a closure and an AST for its definition.
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureAST(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t closureEnvironment, tuuvm_tuple_t argumentSymbols, tuuvm_tuple_t body);

/**
 * Gets the function argument count.
 */
TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function);

/**
 * Gets the function flags.
 */
TUUVM_API size_t tuuvm_function_getFlags(tuuvm_context_t *context, tuuvm_tuple_t function);

/**
 * Is this function a macro?
 */
TUUVM_INLINE bool tuuvm_function_isMacro(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return (tuuvm_function_getFlags(context, function) & TUUVM_FUNCTION_FLAGS_MACRO) != 0;
}

/**
 * Is this function a macro?
 */
TUUVM_INLINE bool tuuvm_function_isVariadic(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return (tuuvm_function_getFlags(context, function) & TUUVM_FUNCTION_FLAGS_VARIADIC) != 0;
}

/**
 * Applies a function tuple with the given parameters
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments);

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply0(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return tuuvm_function_apply(context, function, 0, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply1(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument)
{
    return tuuvm_function_apply(context, function, 1, &argument);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply2(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1
    };
    
    return tuuvm_function_apply(context, function, 2, arguments);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply3(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
    };
    
    return tuuvm_function_apply(context, function, 3, arguments);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply4(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2, tuuvm_tuple_t argument3)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return tuuvm_function_apply(context, function, 4, arguments);
}

#endif //TUUVM_FUNCTION_H
