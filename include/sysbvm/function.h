#ifndef SYSBVM_FUNCTION_H
#define SYSBVM_FUNCTION_H

#pragma once

#include "type.h"
#include "programEntity.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef sysbvm_tuple_t (*sysbvm_functionEntryPoint_t)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);

typedef enum sysbvm_functionFlags_e
{
    SYSBVM_FUNCTION_FLAGS_NONE = 0,
    SYSBVM_FUNCTION_FLAGS_MACRO = 1<<0,
    SYSBVM_FUNCTION_FLAGS_VARIADIC = 1<<1,
    SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE = 1<<2,
    SYSBVM_FUNCTION_FLAGS_PURE = 1<<3,
    SYSBVM_FUNCTION_FLAGS_FINAL = 1<<4,

    SYSBVM_FUNCTION_FLAGS_VIRTUAL = 1<<5,
    SYSBVM_FUNCTION_FLAGS_ABSTRACT = 1<<6,
    SYSBVM_FUNCTION_FLAGS_OVERRIDE = 1<<7,
    SYSBVM_FUNCTION_FLAGS_STATIC = 1<<8,

    SYSBVM_FUNCTION_FLAGS_MEMOIZED = 1<<9,
    SYSBVM_FUNCTION_FLAGS_TEMPLATE = 1<<10,
    SYSBVM_FUNCTION_FLAGS_MEMOIZED_TEMPLATE = SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE,

    SYSBVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS = 1<<11,
    SYSBVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER = 1<<12,

    SYSBVM_FUNCTION_FLAGS_EXTERN_C = 1<<13,
    SYSBVM_FUNCTION_FLAGS_DLLIMPORT = 1<<14,
    SYSBVM_FUNCTION_FLAGS_DLLEXPORT = 1<<15,

    SYSBVM_FUNCTION_FLAGS_INLINE = 1<<16,
    SYSBVM_FUNCTION_FLAGS_ALWAYS_INLINE = 1<<17,
    SYSBVM_FUNCTION_FLAGS_NEVER_INLINE = 1<<18,

    SYSBVM_FUNCTION_FLAGS_VIRTUAL_DISPATCH_FLAGS = SYSBVM_FUNCTION_FLAGS_VIRTUAL | SYSBVM_FUNCTION_FLAGS_ABSTRACT | SYSBVM_FUNCTION_FLAGS_OVERRIDE,

    SYSBVM_FUNCTION_FLAGS_GETTER_FLAGS = SYSBVM_FUNCTION_FLAGS_ALWAYS_INLINE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL,
    SYSBVM_FUNCTION_FLAGS_SETTER_FLAGS = SYSBVM_FUNCTION_FLAGS_ALWAYS_INLINE | SYSBVM_FUNCTION_FLAGS_FINAL,

    SYSBVM_FUNCTION_TYPE_FLAGS = SYSBVM_FUNCTION_FLAGS_VARIADIC | SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE
    
} sysbvm_functionFlags_t;

typedef enum sysbvm_functionApplicationFlags_e
{
    SYSBVM_FUNCTION_APPLICATION_FLAGS_NONE = 0,
    SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK = 1<<0,
    SYSBVM_FUNCTION_APPLICATION_FLAGS_PASS_THROUGH_REFERENCES = 1<<1,
} sysbvm_functionApplicationFlags_t;

typedef struct sysbvm_function_s
{
    sysbvm_programEntity_t super;
    sysbvm_tuple_t flags;
    sysbvm_tuple_t argumentCount;
    sysbvm_tuple_t captureVector;
    sysbvm_tuple_t captureEnvironment;
    sysbvm_tuple_t definition;
    sysbvm_tuple_t primitiveTableIndex;
    sysbvm_tuple_t primitiveName;
    sysbvm_tuple_t memoizationTable;
    sysbvm_tuple_t annotations;
} sysbvm_function_t;

typedef struct sysbvm_functionDefinition_s
{
    sysbvm_programEntity_t super;
    
    sysbvm_tuple_t flags;
    sysbvm_tuple_t argumentCount;
    sysbvm_tuple_t sourcePosition;

    sysbvm_tuple_t definitionEnvironment;
    sysbvm_tuple_t definitionArgumentNodes;
    sysbvm_tuple_t definitionResultTypeNode;
    sysbvm_tuple_t definitionBodyNode;
    
    sysbvm_tuple_t analyzedType;

    sysbvm_tuple_t analysisEnvironment;
    sysbvm_tuple_t analyzedCaptures;
    sysbvm_tuple_t analyzedArguments;
    sysbvm_tuple_t analyzedLocals;
    sysbvm_tuple_t analyzedPragmas;
    sysbvm_tuple_t analyzedInnerFunctions;
    sysbvm_tuple_t analyzedPrimitiveName;

    sysbvm_tuple_t analyzedArgumentNodes;
    sysbvm_tuple_t analyzedResultTypeNode;
    sysbvm_tuple_t analyzedBodyNode;

    sysbvm_tuple_t bytecode;
    sysbvm_tuple_t capturelessUncheckedEntryPoint;
    sysbvm_tuple_t uncheckedEntryPoint;
    sysbvm_tuple_t checkedEntryPoint;
    sysbvm_tuple_t annotations;
} sysbvm_functionDefinition_t;

#define SYSBVM_MAX_FUNCTION_ARGUMENTS 16

typedef struct sysbvm_functionCallFrameStack_s
{
    bool isVariadic;
    bool isMemoizedTemplate;
    size_t expectedArgumentCount;
    size_t variadicArgumentIndex;
    size_t argumentIndex;

    struct {
        sysbvm_tuple_t function;
        sysbvm_tuple_t applicationArguments[SYSBVM_MAX_FUNCTION_ARGUMENTS];
    } gcRoots;
} sysbvm_functionCallFrameStack_t;

/**
 * Creates a function definition.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionDefinition_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t flags, sysbvm_tuple_t argumentCount, sysbvm_tuple_t definitionEnvironment, sysbvm_tuple_t argumentNodes, sysbvm_tuple_t resultTypeNode, sysbvm_tuple_t body);

/**
 * Creates a primitive function tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_function_createPrimitive(sysbvm_context_t *context, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint);

/**
 * Creates a closure by passing its definition and capture vector.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_function_createClosureWithCaptureVector(sysbvm_context_t *context, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captureVector);

/**
 * Creates a closure with lazy analysis by passing its capture environment.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_function_createClosureWithCaptureEnvironment(sysbvm_context_t *context, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captureEnvironment);

/**
 * Ensures the completion of a lazy function analysis.
 */
SYSBVM_API void sysbvm_function_ensureAnalysis(sysbvm_context_t *context, sysbvm_function_t **function);

/**
 * Ensures the completion of a function definition.
 */
SYSBVM_API void sysbvm_functionDefinition_ensureAnalysis(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition);

/**
 * Ensures the completion of a function definition type analysis.
 */
SYSBVM_API void sysbvm_functionDefinition_ensureTypeAnalysis(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition);

/**
 * Gets the function argument count.
 */
SYSBVM_API size_t sysbvm_function_getArgumentCount(sysbvm_context_t *context, sysbvm_tuple_t function);

/**
 * Gets the function flags.
 */
SYSBVM_INLINE sysbvm_bitflags_t sysbvm_function_getFlags(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return sysbvm_tuple_isFunction(context, function) ? sysbvm_tuple_bitflags_decode(((sysbvm_function_t*)function)->flags) : 0;
}

/**
 * Sets the function flags.
 */
SYSBVM_API void sysbvm_function_setFlags(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_bitflags_t flags);

/**
 * Add flags to the function.
 */
SYSBVM_API void sysbvm_function_addFlags(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_bitflags_t flags);

/**
 * Is this function a macro?
 */
SYSBVM_INLINE bool sysbvm_function_isMacro(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & SYSBVM_FUNCTION_FLAGS_MACRO) != 0;
}

/**
 * Is this function a macro?
 */
SYSBVM_INLINE bool sysbvm_function_isVariadic(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & SYSBVM_FUNCTION_FLAGS_VARIADIC) != 0;
}

/**
 * Is this a pure function?
 */
SYSBVM_INLINE bool sysbvm_function_isPure(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & SYSBVM_FUNCTION_FLAGS_PURE) != 0;
}

/**
 * Is this a memoized function?
 */
SYSBVM_INLINE bool sysbvm_function_isMemoized(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & SYSBVM_FUNCTION_FLAGS_MEMOIZED) != 0;
}

/**
 * Is this a template function?
 */
SYSBVM_INLINE bool sysbvm_function_isTemplate(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & SYSBVM_FUNCTION_FLAGS_TEMPLATE) != 0;
}

/**
 * Is this a memoized template function?
 */
SYSBVM_INLINE bool sysbvm_function_isMemoizedTemplate(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return (sysbvm_function_getFlags(context, function) & (SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE)) == (SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE);
}

/**
 * Applies a function tuple with the given parameters
 */
SYSBVM_API sysbvm_tuple_t sysbvm_function_apply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags);

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_apply0(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return sysbvm_function_apply(context, function, 0, 0, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_apply1(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument)
{
    return sysbvm_function_apply(context, function, 1, &argument, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_apply2(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1
    };
    
    return sysbvm_function_apply(context, function, 2, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_apply3(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
    };
    
    return sysbvm_function_apply(context, function, 3, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_apply4(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2, sysbvm_tuple_t argument3)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return sysbvm_function_apply(context, function, 4, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_applyNoCheck0(sysbvm_context_t *context, sysbvm_tuple_t function)
{
    return sysbvm_function_apply(context, function, 0, 0, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_applyNoCheck1(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument)
{
    return sysbvm_function_apply(context, function, 1, &argument, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_applyNoCheck2(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1
    };
    
    return sysbvm_function_apply(context, function, 2, arguments, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_applyNoCheck3(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
    };
    
    return sysbvm_function_apply(context, function, 3, arguments, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_function_applyNoCheck4(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2, sysbvm_tuple_t argument3)
{
    sysbvm_tuple_t arguments[] = {
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return sysbvm_function_apply(context, function, 4, arguments, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
}
/**
 * Sends a message with the given selector.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_send(sysbvm_context_t *context, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *arguments, uint32_t applicationFlags);

SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_send0(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t receiver)
{
    return sysbvm_tuple_send(context, selector, 1, &receiver, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_send1(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t argument)
{
    sysbvm_tuple_t arguments[] = {
        receiver,
        argument
    };
    return sysbvm_tuple_send(context, selector, 2, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_send2(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1)
{
    sysbvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1
    };
    
    return sysbvm_tuple_send(context, selector, 3, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_send3(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2)
{
    sysbvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1,
        argument2,
    };
    
    return sysbvm_tuple_send(context, selector, 4, arguments, 0);
}

SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_send4(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t argument2, sysbvm_tuple_t argument3)
{
    sysbvm_tuple_t arguments[] = {
        receiver,
        argument0,
        argument1,
        argument2,
        argument3,
    };
    
    return sysbvm_tuple_send(context, selector, 5, arguments, 0);
}

/**
 * Begins constructing a function call frame stack.
 */
SYSBVM_API void sysbvm_functionCallFrameStack_begin(sysbvm_context_t *context, sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_tuple_t function, size_t argumentCount);

/**
 * Pushes an argument into the function call frame stack.
 */
SYSBVM_API void sysbvm_functionCallFrameStack_push(sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_tuple_t argument);

/**
 * Ends constructing a function call frame stack and dispatch the function call.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionCallFrameStack_finish(sysbvm_context_t *context, sysbvm_functionCallFrameStack_t *callFrameStack, sysbvm_bitflags_t applicationFlags);

/**
 * Does the function require an optimized lookup?
 */
SYSBVM_API bool sysbvm_function_shouldOptimizeLookup(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t receiverType, bool hasLiteralReceiver);

/**
 * Gets the entry point of the numbered primitive.
 */
SYSBVM_API sysbvm_functionEntryPoint_t sysbvm_function_getNumberedPrimitiveEntryPoint(sysbvm_context_t *context, uint32_t primitiveNumber);

/**
 * Records a specific function binding with its owner and its name.
 */
SYSBVM_API void sysbvm_function_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t function, sysbvm_tuple_t owner, sysbvm_tuple_t name);

#endif //SYSBVM_FUNCTION_H
