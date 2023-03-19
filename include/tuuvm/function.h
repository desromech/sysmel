#ifndef TUUVM_FUNCTION_H
#define TUUVM_FUNCTION_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef tuuvm_tuple_t (*tuuvm_functionEntryPoint_t)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);

typedef enum tuuvm_functionFlags_e
{
    TUUVM_FUNCTION_FLAGS_NONE = 0,
    TUUVM_FUNCTION_FLAGS_MACRO = 1<<0,
    TUUVM_FUNCTION_FLAGS_VARIADIC = 1<<1,
    TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE = 1<<2,
    TUUVM_FUNCTION_FLAGS_PURE = 1<<3,
    TUUVM_FUNCTION_FLAGS_FINAL = 1<<4,

    TUUVM_FUNCTION_FLAGS_VIRTUAL = 1<<5,
    TUUVM_FUNCTION_FLAGS_ABSTRACT = 1<<6,
    TUUVM_FUNCTION_FLAGS_OVERRIDE = 1<<7,
    TUUVM_FUNCTION_FLAGS_STATIC = 1<<8,

    TUUVM_FUNCTION_FLAGS_MEMOIZED = 1<<9,
    TUUVM_FUNCTION_FLAGS_TEMPLATE = 1<<10,

    TUUVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS = 1<<11,
    TUUVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER = 1<<12,

    TUUVM_FUNCTION_FLAGS_GETTER_FLAGS = TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL,
    TUUVM_FUNCTION_FLAGS_SETTER_FLAGS = TUUVM_FUNCTION_FLAGS_FINAL,
} tuuvm_functionFlags_t;

typedef enum tuuvm_functionApplicationFlags_e
{
    TUUVM_FUNCTION_APPLICATION_FLAGS_NONE = 0,
    TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK = 1<<0,
    TUUVM_FUNCTION_APPLICATION_FLAGS_PASS_THROUGH_REFERENCES = 1<<1,
} tuuvm_functionApplicationFlags_t;

typedef struct tuuvm_function_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t owner;
    tuuvm_tuple_t flags;
    tuuvm_tuple_t argumentCount;
    tuuvm_tuple_t captureVector;
    tuuvm_tuple_t definition;
    tuuvm_tuple_t primitiveTableIndex;
    tuuvm_tuple_t primitiveName;
    tuuvm_tuple_t nativeUserdata;
    tuuvm_tuple_t nativeEntryPoint;
    tuuvm_tuple_t memoizationTable;
} tuuvm_function_t;

typedef struct tuuvm_functionDefinition_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t flags;
    tuuvm_tuple_t argumentCount;
    tuuvm_tuple_t sourcePosition;

    tuuvm_tuple_t definitionEnvironment;
    tuuvm_tuple_t definitionArgumentNodes;
    tuuvm_tuple_t definitionResultTypeNode;
    tuuvm_tuple_t definitionBodyNode;
    
    tuuvm_tuple_t analyzedType;

    tuuvm_tuple_t analysisEnvironment;
    tuuvm_tuple_t analyzedCaptures;
    tuuvm_tuple_t analyzedArguments;
    tuuvm_tuple_t analyzedLocals;
    tuuvm_tuple_t analyzedPragmas;
    tuuvm_tuple_t analyzedPrimitiveName;

    tuuvm_tuple_t analyzedArgumentNodes;
    tuuvm_tuple_t analyzedResultTypeNode;
    tuuvm_tuple_t analyzedBodyNode;
} tuuvm_functionDefinition_t;

#define TUUVM_MAX_FUNCTION_ARGUMENTS 16

typedef struct tuuvm_functionCallFrameStack_s
{
    bool isVariadic;
    size_t expectedArgumentCount;
    size_t variadicArgumentIndex;
    size_t argumentIndex;

    struct {
        tuuvm_tuple_t function;
        tuuvm_tuple_t applicationArguments[TUUVM_MAX_FUNCTION_ARGUMENTS];
    } gcRoots;
} tuuvm_functionCallFrameStack_t;

/**
 * Creates a function definition.
 */
TUUVM_API tuuvm_tuple_t tuuvm_functionDefinition_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t argumentCount, tuuvm_tuple_t definitionEnvironment, tuuvm_tuple_t argumentNodes, tuuvm_tuple_t resultTypeNode, tuuvm_tuple_t body);

/**
 * Creates a primitive function tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

/**
 * Creates a closure by passing its definition and environment.
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureWithCaptureVector(tuuvm_context_t *context, tuuvm_tuple_t functionDefinition, tuuvm_tuple_t captureVector);

/**
 * Gets the function argument count.
 */
TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function);

/**
 * Gets the function flags.
 */
TUUVM_API size_t tuuvm_function_getFlags(tuuvm_context_t *context, tuuvm_tuple_t function);

/**
 * Sets the function flags.
 */
TUUVM_API void tuuvm_function_setFlags(tuuvm_context_t *context, tuuvm_tuple_t function, size_t flags);

/**
 * Add flags to the function.
 */
TUUVM_API void tuuvm_function_addFlags(tuuvm_context_t *context, tuuvm_tuple_t function, size_t flags);

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
 * Is this a pure function?
 */
TUUVM_INLINE bool tuuvm_function_isPure(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return (tuuvm_function_getFlags(context, function) & TUUVM_FUNCTION_FLAGS_PURE) != 0;
}

/**
 * Is this a memoized function?
 */
TUUVM_INLINE bool tuuvm_function_isMemoized(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return (tuuvm_function_getFlags(context, function) & TUUVM_FUNCTION_FLAGS_MEMOIZED) != 0;
}

/**
 * Applies a function tuple with the given parameters
 */
TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags);

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply0(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return tuuvm_function_apply(context, function, 0, 0, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply1(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument)
{
    return tuuvm_function_apply(context, function, 1, &argument, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply2(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1
    };
    
    return tuuvm_function_apply(context, function, 2, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply3(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
    };
    
    return tuuvm_function_apply(context, function, 3, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_apply4(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2, tuuvm_tuple_t argument3)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return tuuvm_function_apply(context, function, 4, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_applyNoCheck0(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    return tuuvm_function_apply(context, function, 0, 0, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_applyNoCheck1(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument)
{
    return tuuvm_function_apply(context, function, 1, &argument, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_applyNoCheck2(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1
    };
    
    return tuuvm_function_apply(context, function, 2, arguments, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_applyNoCheck3(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
    };
    
    return tuuvm_function_apply(context, function, 3, arguments, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_function_applyNoCheck4(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2, tuuvm_tuple_t argument3)
{
    tuuvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return tuuvm_function_apply(context, function, 4, arguments, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}
/**
 * Sends a message with the given selector.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_send(tuuvm_context_t *context, tuuvm_tuple_t selector, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags);

TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_send0(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t receiver)
{
    return tuuvm_tuple_send(context, selector, 1, &receiver, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_send1(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t argument)
{
    tuuvm_tuple_t arguments[] = {
        receiver,
        argument
    };
    return tuuvm_tuple_send(context, selector, 2, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_send2(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1)
{
    tuuvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1
    };
    
    return tuuvm_tuple_send(context, selector, 3, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_send3(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2)
{
    tuuvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1,
        argument2,
    };
    
    return tuuvm_tuple_send(context, selector, 4, arguments, 0);
}

TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_send4(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t argument2, tuuvm_tuple_t argument3)
{
    tuuvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return tuuvm_tuple_send(context, selector, 5, arguments, 0);
}

/**
 * Begins constructing a function call frame stack.
 */
TUUVM_API void tuuvm_functionCallFrameStack_begin(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t function, size_t argumentCount);

/**
 * Pushes an argument into the function call frame stack.
 */
TUUVM_API void tuuvm_functionCallFrameStack_push(tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t argument);

/**
 * Ends constructing a function call frame stack and dispatch the function call.
 */
TUUVM_API tuuvm_tuple_t tuuvm_functionCallFrameStack_finish(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack, uint32_t applicationFlags);

/**
 * Does the function require an optimized lookup?
 */
TUUVM_API bool tuuvm_function_shouldOptimizeLookup(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t receiverType, bool hasLiteralReceiver);

#endif //TUUVM_FUNCTION_H
