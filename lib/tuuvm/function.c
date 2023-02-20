#include "tuuvm/function.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/errors.h"
#include "tuuvm/interpreter.h"
#include "tuuvm/stackFrame.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    tuuvm_primitiveFunction_t *result = (tuuvm_primitiveFunction_t*)tuuvm_context_allocateByteTuple(context, context->roots.primitiveFunctionType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_primitiveFunction_t));
    result->argumentCount = argumentCount;
    result->flags = flags;
    result->userdata = userdata;
    result->entryPoint = entryPoint;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureAST(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t closureEnvironment, tuuvm_tuple_t argumentSymbols, tuuvm_tuple_t body)
{
    tuuvm_closureASTFunction_t *result = (tuuvm_closureASTFunction_t*)tuuvm_context_allocateByteTuple(context, context->roots.closureASTFunctionType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_closureASTFunction_t));
    result->sourcePosition = sourcePosition;
    result->flags = flags;
    result->closureEnvironment = closureEnvironment;
    result->argumentSymbols = argumentSymbols;
    result->body = body;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->argumentCount;
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        tuuvm_closureASTFunction_t *closureASTFunction = (tuuvm_closureASTFunction_t*)function;
        return tuuvm_arraySlice_getSize(closureASTFunction->argumentSymbols);
    }

    return 0;
}

TUUVM_API size_t tuuvm_function_getFlags(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->flags;
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        tuuvm_closureASTFunction_t *closureASTFunction = (tuuvm_closureASTFunction_t*)function;
        return tuuvm_tuple_size_decode(closureASTFunction->flags);
    }

    return 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);

    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        tuuvm_tuple_t result = primitiveFunction->entryPoint(context, function, argumentCount, arguments);
        tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
        return result;
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        tuuvm_tuple_t result = tuuvm_interpreter_applyClosureASTFunction(context, function, argumentCount, arguments);
        tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
        return result;
    }

    tuuvm_error("Cannot apply non-functional object.");
    return TUUVM_VOID_TUPLE;
}

TUUVM_API void tuuvm_functionCallFrameStack_begin(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t function, size_t argumentCount)
{
    callFrameStack->gcRoots.function = function;
    callFrameStack->isVariadic = tuuvm_function_isVariadic(context, callFrameStack->gcRoots.function);
    callFrameStack->expectedArgumentCount = tuuvm_function_getArgumentCount(context, callFrameStack->gcRoots.function);
    callFrameStack->argumentIndex = 0;
    callFrameStack->variadicArgumentIndex = 0;

    size_t requiredArgumentCount = callFrameStack->expectedArgumentCount;
    if(callFrameStack->isVariadic)
    {
        if(requiredArgumentCount == 0)
            tuuvm_error("Variadic functions require at least a single argument.");
        callFrameStack->variadicArgumentIndex = requiredArgumentCount - 1;

        --requiredArgumentCount;
        if(argumentCount < requiredArgumentCount)
            tuuvm_error("Missing required argument count.");

        size_t variadicArgumentCount = argumentCount - requiredArgumentCount;
        callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex] = tuuvm_arraySlice_createWithArrayOfSize(context, variadicArgumentCount);
    }
    else
    {
        if(argumentCount != requiredArgumentCount)
            tuuvm_error("Function call does not receive the required number of arguments.");
    }

    if(argumentCount > TUUVM_MAX_FUNCTION_ARGUMENTS && !callFrameStack->isVariadic)
        tuuvm_error("Function application direct arguments exceeds the max argument count.");
    
}

TUUVM_API void tuuvm_functionCallFrameStack_push(tuuvm_functionCallFrameStack_t *callFrameStack, tuuvm_tuple_t argument)
{
    if(!callFrameStack->isVariadic || callFrameStack->argumentIndex < callFrameStack->variadicArgumentIndex)
    {
        callFrameStack->gcRoots.applicationArguments[callFrameStack->argumentIndex++] = argument;
        return;
    }

    tuuvm_arraySlice_atPut(callFrameStack->gcRoots.applicationArguments[callFrameStack->variadicArgumentIndex], callFrameStack->argumentIndex - callFrameStack->variadicArgumentIndex, argument);
    ++callFrameStack->argumentIndex;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionCallFrameStack_finish(tuuvm_context_t *context, tuuvm_functionCallFrameStack_t *callFrameStack)
{
    return tuuvm_function_apply(context, callFrameStack->gcRoots.function, callFrameStack->expectedArgumentCount, callFrameStack->gcRoots.applicationArguments);
}

static tuuvm_tuple_t tuuvm_function_primitive_apply(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    tuuvm_tuple_t *argumentList = &arguments[1];

    size_t variadicArgumentCount = tuuvm_arraySlice_getSize(*argumentList);
    size_t argumentListSize = 0;
    size_t callArgumentCount = 0;
    if(variadicArgumentCount > 0)
    {
        argumentListSize = tuuvm_arraySlice_getSize(tuuvm_arraySlice_at(*argumentList, variadicArgumentCount - 1));
        callArgumentCount = variadicArgumentCount - 1 + argumentListSize;
    }

    tuuvm_functionCallFrameStack_t callFrameStack = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    tuuvm_functionCallFrameStack_begin(context, &callFrameStack, *function, callArgumentCount);
    if(variadicArgumentCount > 0)
    {
        for(size_t i = 0; i < variadicArgumentCount - 1; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_arraySlice_at(*argumentList, i));
        
        tuuvm_tuple_t argumentArraySlice = tuuvm_arraySlice_at(*argumentList, variadicArgumentCount - 1);
        for(size_t i = 0; i < argumentListSize; ++i)
            tuuvm_functionCallFrameStack_push(&callFrameStack, tuuvm_arraySlice_at(argumentArraySlice, i));
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    return tuuvm_functionCallFrameStack_finish(context, &callFrameStack);
}

void tuuvm_function_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "apply", 2, TUUVM_FUNCTION_FLAGS_VARIADIC, NULL, tuuvm_function_primitive_apply);
}
