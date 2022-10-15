#ifndef TUUVM_FUNCTION_H
#define TUUVM_FUNCTION_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef tuuvm_tuple_t (*tuuvm_functionEntryPoint_t)(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments);

typedef struct tuuvm_primitiveFunction_s
{
    tuuvm_tuple_header_t header;
    void *userdata;
    tuuvm_functionEntryPoint_t entryPoint;
} tuuvm_primitiveFunction_t;


#define TUUVM_MAX_FUNCTION_ARGUMENTS 16

/**
 * Creates a primitive function tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

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
