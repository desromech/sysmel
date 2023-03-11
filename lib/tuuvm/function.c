#include "tuuvm/function.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/errors.h"
#include "tuuvm/interpreter.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdlib.h>
#include <stdio.h>

#define PRIMITIVE_TABLE_CAPACITY 1024

static bool tuuvm_primitiveTableIsComputed = false;
static size_t tuuvm_primitiveTableSize = 0;
static tuuvm_functionEntryPoint_t tuuvm_primitiveTable[PRIMITIVE_TABLE_CAPACITY];
extern void tuuvm_context_registerPrimitives(void);
extern tuuvm_tuple_t tuuvm_interpreter_recompileAndOptimizeFunction(tuuvm_context_t *context, tuuvm_function_t **functionObject);

void tuuvm_primitiveTable_registerFunction(tuuvm_functionEntryPoint_t primitiveEntryPoint)
{
    for(size_t i = 0; i < tuuvm_primitiveTableSize; ++i)
    {
        if(tuuvm_primitiveTable[i] == primitiveEntryPoint)
            return;
    }

    tuuvm_primitiveTable[tuuvm_primitiveTableSize++] = primitiveEntryPoint;
}

static void tuuvm_primitiveTable_ensureIsComputed(void)
{
    if(tuuvm_primitiveTableIsComputed)
        return;

    tuuvm_primitiveTableIsComputed = true;
    tuuvm_context_registerPrimitives();
}

static bool tuuvm_primitiveTable_findEntryFor(tuuvm_functionEntryPoint_t primitiveEntryPoint, size_t *outEntryIndex)
{
    tuuvm_primitiveTable_ensureIsComputed();
    
    for(size_t i = 0; i < tuuvm_primitiveTableSize; ++i)
    {
        if(tuuvm_primitiveTable[i] == primitiveEntryPoint)
        {
            *outEntryIndex = i;
            return true;
        }
    }
    return false;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    tuuvm_function_t *result = (tuuvm_function_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_function_t));
    result->argumentCount = tuuvm_tuple_size_encode(context, argumentCount);
    result->flags = tuuvm_tuple_size_encode(context, flags);
    size_t primitiveEntryIndex = 0;
    if(!userdata && tuuvm_primitiveTable_findEntryFor(entryPoint, &primitiveEntryIndex))
    {
        result->primitiveTableIndex = tuuvm_tuple_uintptr_encode(context, primitiveEntryIndex);
    }
    else
    {
        result->nativeUserdata = tuuvm_tuple_uintptr_encode(context, (size_t)userdata);
        result->nativeEntryPoint = tuuvm_tuple_uintptr_encode(context, (size_t)entryPoint);
    }
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureAST(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t argumentCount, tuuvm_tuple_t closureEnvironment, tuuvm_tuple_t argumentNodes, tuuvm_tuple_t resultTypeNode, tuuvm_tuple_t body)
{
    tuuvm_function_t *result = (tuuvm_function_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_function_t));
    result->sourcePosition = sourcePosition;
    result->flags = flags;
    result->argumentCount = argumentCount; 
    result->closureEnvironment = closureEnvironment;
    result->argumentNodes = argumentNodes;
    result->resultTypeNode = resultTypeNode;
    result->body = body;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    if(tuuvm_tuple_isKindOf(context, function, context->roots.functionType))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
        return tuuvm_tuple_size_decode(functionObject->argumentCount);
    }

    return 0;
}

TUUVM_API size_t tuuvm_function_getFlags(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    if(tuuvm_tuple_isKindOf(context, function, context->roots.functionType))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
        return tuuvm_tuple_size_decode(functionObject->flags);
    }

    return 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    struct {
        tuuvm_tuple_t function;
        tuuvm_tuple_t functionType;
        tuuvm_tuple_t result;
    } gcFrame = {
        .function = function
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_stackFrameGCRootsRecord_t argumentsRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS,
        .rootCount = argumentCount,
        .roots = arguments
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);

    gcFrame.functionType = tuuvm_tuple_getType(context, function);
    if(tuuvm_tuple_isKindOf(context, function, context->roots.functionType))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
        tuuvm_functionEntryPoint_t nativeEntryPoint = NULL;
        
        // Find the entry point in the primitive table.
        if(functionObject->primitiveTableIndex)
        {
            tuuvm_primitiveTable_ensureIsComputed();
            size_t primitiveNumber = tuuvm_tuple_size_decode(functionObject->primitiveTableIndex);
            if(primitiveNumber < tuuvm_primitiveTableSize)
                nativeEntryPoint = tuuvm_primitiveTable[primitiveNumber];
        }

        if(!nativeEntryPoint)
            nativeEntryPoint = (tuuvm_functionEntryPoint_t)tuuvm_tuple_uintptr_decode(functionObject->nativeEntryPoint);
        if(nativeEntryPoint)
        {
            gcFrame.result = nativeEntryPoint(context, &gcFrame.function, argumentCount, arguments);
            tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;            
        }
        else if(functionObject->body)
        {
            gcFrame.result = tuuvm_interpreter_applyClosureASTFunction(context, &gcFrame.function, argumentCount, arguments);
            tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&argumentsRecord);
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }

    printf("functionType %p expected type %p\n", (void*)gcFrame.functionType, (void*)context->roots.functionType);
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

static tuuvm_tuple_t tuuvm_function_primitive_apply(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

    tuuvm_functionCallFrameStack_t callFrameStack = {0};
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

static tuuvm_tuple_t tuuvm_function_primitive_isCorePrimitive(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    if(!tuuvm_tuple_isKindOf(context, *function, context->roots.functionType))
        return TUUVM_FALSE_TUPLE;
    
    tuuvm_function_t *functionObject = (tuuvm_function_t*)*function;
    size_t flags = tuuvm_tuple_size_decode(functionObject->flags);
    return tuuvm_tuple_boolean_encode((flags & TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE) != 0);
}

static tuuvm_tuple_t tuuvm_function_primitive_adoptDefinitionOf(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    tuuvm_tuple_t *definitionFunction = &arguments[0];
    if(!tuuvm_tuple_isKindOf(context, *function, context->roots.functionType)) tuuvm_error("Expected a function.");
    if(!tuuvm_tuple_isKindOf(context, *definitionFunction, context->roots.functionType)) tuuvm_error("Expected a function.");
    
    tuuvm_function_t *functionObject = (tuuvm_function_t*)*function;
    tuuvm_function_t *definitionFunctionObject = (tuuvm_function_t*)*definitionFunction;

    functionObject->sourcePosition = definitionFunctionObject->sourcePosition;
    functionObject->closureEnvironment = definitionFunctionObject->closureEnvironment;
    functionObject->argumentNodes = definitionFunctionObject->argumentNodes;
    functionObject->resultTypeNode = definitionFunctionObject->resultTypeNode;
    functionObject->body = definitionFunctionObject->body;

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_function_primitive_recompileAndOptimize(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *function = &arguments[0];
    if(!tuuvm_tuple_isKindOf(context, *function, context->roots.functionType)) tuuvm_error("Expected a function.");
    
    tuuvm_function_t **functionObject = (tuuvm_function_t**)function;
    if((*functionObject)->body && (*functionObject)->closureEnvironment)
        return tuuvm_interpreter_recompileAndOptimizeFunction(context, functionObject);
    
    return *function;
}

bool tuuvm_function_shouldOptimizeLookup(tuuvm_context_t *context, tuuvm_tuple_t function, tuuvm_tuple_t receiverType)
{
    (void)receiverType;
    return
        (tuuvm_type_getFlags(receiverType) & (TUUVM_TYPE_FLAGS_FINAL)) != TUUVM_TYPE_FLAGS_NONE ||
        (tuuvm_function_getFlags(context, function) & (TUUVM_FUNCTION_FLAGS_MACRO | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL)) != TUUVM_FUNCTION_FLAGS_NONE;
}

void tuuvm_function_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_apply);
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_adoptDefinitionOf);
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_isCorePrimitive);
    tuuvm_primitiveTable_registerFunction(tuuvm_function_primitive_recompileAndOptimize);
}

void tuuvm_function_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "apply", context->roots.functionType, "applyWithArguments:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_VARIADIC | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_function_primitive_apply);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Function::adoptDefinitionOf:", context->roots.functionType, "adoptDefinitionOf:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_function_primitive_adoptDefinitionOf);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Function::isCorePrimitive", context->roots.functionType, "isCorePrimitive", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_function_primitive_isCorePrimitive);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Function::recompileAndOptimize", context->roots.functionType, "recompileAndOptimize", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_function_primitive_recompileAndOptimize);
}
